//
// Created by marcel on 03.12.16.
//

#include "CEGIS.h"

// ***********************************************************
// ******************* CEGISHandler **************************
// ***********************************************************

size_t CEGISHandler::ceID = 0;

CEGISHandler::CEGISHandler(z3::context   * const   ctx,
                           z3::expr_vector const & implVars,
                           z3::expr_vector const & inpVars,
                           z3::expr_vector const & hlpVars,
                           z3::expr        const & implExpr,
                           z3::expr        const & behavExpr,
                           z3::expr        const & corrExpr)
        :
        context{ctx},
        implementationVariables{implVars},
        inputVariables{inpVars},
        helperVariables{hlpVars},
        implementationExpression{implExpr},
        behavioralExpression{behavExpr},
        correctnessExpression{corrExpr},
        implementationSolver{*context},
        counterExampleSolver{*context},
        counterExamples{}
{}

const CEGISHandler::CEGISResult CEGISHandler::CEGISRoutine()
{
    implementationSolver.add(implementationExpression);
    counterExampleSolver.add(behavioralExpression && !correctnessExpression);

    auto start = clock::now();
    while (true) {
        auto implTp = findImplementation();
        if (getResult(implTp) == z3::sat) // another implementation was found
        {
            auto ceTp = findCounterExample(getImpl(implTp));
            if (getResult(ceTp) == z3::sat) // another counter-example was found
            {
                counterExamples.push_back(std::move(getCE(ceTp)));
            } else // no more counter-examples possible
            {
                return CEGISResult(implTp, counterExamples, start, clock::now(), name);
            }
        } else // no implementation possible
        {
            return CEGISResult(implTp, counterExamples, start, clock::now(), name);
        }
    }
}

const CEGISHandler::ImplementationPair CEGISHandler::findImplementation()
{
    if (!counterExamples.empty()) {
        // extract constraints from latest counter example
        auto counterExampleCons = counterExamples.back()
                .extractConstraints(inputVariables, INPUT_VAR_TEMPLATE, context);

        // substitute variables in behavior and correctness expression
        auto subExpr = substituteVars(behavioralExpression && correctnessExpression,
                                      inputVariables, INPUT_VAR_TEMPLATE);
        subExpr = substituteVars(subExpr, helperVariables, HELPER_VAR_TEMPLATE);

        implementationSolver.add(counterExampleCons && subExpr);
    }
    switch (implementationSolver.check())
    {
        case z3::sat:
        {
            auto m = implementationSolver.get_model();
            return std::make_pair(Implementation(m), z3::sat);
        }
        case z3::unsat:
            return std::make_pair(boost::optional<Implementation>(), z3::unsat);
        case z3::unknown:
            return std::make_pair(boost::optional<Implementation>(), z3::unknown);
    }
}

const CEGISHandler::CounterExamplePair CEGISHandler::findCounterExample(const Implementation & impl)
{
    auto implCons = impl.extractConstraints(implementationVariables, context);
    counterExampleSolver.push();
    counterExampleSolver.add(implCons);
    switch (counterExampleSolver.check())
    {
        case z3::sat:
        {
            auto m = counterExampleSolver.get_model();
            counterExampleSolver.pop();
            return std::make_pair(CounterExample(m), z3::sat);
        }
        case z3::unsat:
            return std::make_pair(boost::optional<CounterExample>(), z3::unsat);
        case z3::unknown:
            return std::make_pair(boost::optional<CounterExample>(), z3::unknown);
    }
}

const z3::expr CEGISHandler::substituteVars(const z3::expr        & expr,
                                            const z3::expr_vector & vars,
                                                  boost::format     templ) const
{
    z3::expr_vector sub_vars{*context};
    for (auto i = 0; i < vars.size(); ++i)
    {
        z3::expr current = vars[i];
        const char *name = (templ % i % ceID).str().c_str();
        z3::expr e = context->constant(name, current.get_sort());
        sub_vars.push_back(e);
    }
    return z3::expr(expr).substitute(vars, sub_vars);
}

void CEGISHandler::setName(std::string n) { name = n; }


// ************************************************************
// ******************* Implementation *************************
// ************************************************************

CEGISHandler::Implementation::Implementation(const z3::model & mdl)
        :
        model(mdl)
{}

const z3::expr CEGISHandler::Implementation::extractConstraints(const z3::expr_vector & implVars,
                                                                      z3::context     * ctx) const
{
    z3::expr_vector val{*ctx};
    for (auto i = 0; i < implVars.size(); ++i) {
        z3::expr var = implVars[i];
        val.push_back(var == model.eval(var));
    }
    return z3::mk_and(val);
}

const z3::expr CEGISHandler::Implementation::getValuation(z3::expr var) const { return model.eval(var); }


// ************************************************************
// ******************* CounterExample *************************
// ************************************************************

CEGISHandler::CounterExample::CounterExample(const z3::model & mdl)
        :
        model(mdl),
        id(ceID++)
{}

CEGISHandler::CounterExample::CounterExample(CounterExample && ce)
        :
        model(std::move(ce.model)),
        id(std::move(ce.id))
{}

const z3::expr CEGISHandler::CounterExample::extractConstraints(const z3::expr_vector & inputVars,
                                                                      boost::format     templ,
                                                                      z3::context     * ctx) const
{
    z3::expr_vector val{*ctx};
    for (auto i = 0; i < inputVars.size(); ++i)
    {
        z3::expr var = inputVars[i];
        const char *name = (templ % i % id).str().c_str();
        val.push_back(ctx->constant(name, var.get_sort()) == model.eval(var));
    }
    return z3::mk_and(val);
}

const size_t CEGISHandler::CounterExample::getNumber() const { return id; }



// ************************************************************
// ********************* CEGISResult **************************
// ************************************************************

CEGISHandler::CEGISResult::CEGISResult(const ImplementationPair          & implP,
                                       const std::vector<CounterExample> & ces,
                                       const CEGISResult::TimePoint      & start,
                                       const CEGISResult::TimePoint      & end,
                                       const std::string                 & n)
        :
        implementation(getOImpl(implP)),
        result(getResult(implP)),
        counterExamples(ces),
        startPoint(start),
        endPoint(end),
        name(n)
{}

const z3::check_result CEGISHandler::CEGISResult::check() const { return result; }

const z3::expr CEGISHandler::CEGISResult::getValuation(const z3::expr var) const
{
    assert(check() == z3::sat);
    return implementation.get().getValuation(var);
}

const size_t CEGISHandler::CEGISResult::getNumberOfCounterExamples() const { return counterExamples.size(); }

long CEGISHandler::CEGISResult::getRuntime() const
{
    return std::chrono::duration_cast<std::chrono::milliseconds>( endPoint - startPoint ).count();
}

void CEGISHandler::CEGISResult::print(std::ostream &out, bool csv)
{
    if (csv)
        out << name << ", " << result << ", " << getNumberOfCounterExamples() << ", " << getRuntime() << std::endl;
    else
    {
        out << "Benchmark:         " << name                            << std::endl;
        out << "Result:            " << result                          << std::endl;
        out << "#Counter-examples: " << getNumberOfCounterExamples()    << std::endl;
        out << "Runtime:           " << getRuntime() << " milliseconds" << std::endl;
    }
}
