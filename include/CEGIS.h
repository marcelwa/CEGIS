//
// Created by marcel on 03.12.16.
//

#ifndef CEGIS_CEGIS_H
#define CEGIS_CEGIS_H

#include <iostream>
#include <tuple>
#include <chrono>
#include <boost/optional.hpp>
#include <boost/format.hpp>
#include <z3++.h>

/**
 * Handler for Counter Example Guided Inductive Synthesis (CEGIS) routine for the SMT solver z3.
 */
class CEGISHandler {

private:
    /**
     * A class representing an implementation, i.e. a valuation to the implementation variables.
     */
    class Implementation {

    private:
        /**
         * The given model to store the valuation.
         */
        const z3::model model;

    public:
        /**
         * The constructor. Takes a model to store the current valuation determined by the implementationSolver.
         *
         * @param mdl A model representing the implementation valuation.
         */
        Implementation(const z3::model & mdl);

        /**
         * Extracts constraints from the stored model concerning the implementation variables, i.e. extracts an
         * expression stating that the given implementation variables should have the values they have in the model.
         *
         * @param implVars The variables for which the valuation is sought.
         * @return A constraint forcing implVars to have the same valuation as they originally had in the stored
         *         model.
         */
        const z3::expr extractConstraints(const z3::expr_vector & implVars,
                                                z3::context     * ctx) const;
        /**
         * Returns the valuation to the given variable. Returns the variable itself iff no valuation exists.
         *
         * @return The valuation to the given (implementation) variables.
         */
        const z3::expr getValuation(z3::expr var) const;
    }; // Implementation

    /**
     * Class representing a counter example within the CEGIS routine, i.e. a valuation to the input variables
     * so that the current implementation works incorrectly.
     */
    class CounterExample {

    private:
        /**
         * The given model to store the valuation.
         */
        const z3::model model;
        /**
         * A serial number.
         */
        const size_t id;

    public:
        /**
         * Constructor of the CounterExample. Takes a model to store the current valuation determined by
         * the counterExampleSolver.
         *
         * @param mdl A model representing the counter example valuation.
         */
        CounterExample(const z3::model & mdl);
        /**
         * Default copy constructor.
         *
         * @param ce CounterExample to be copied.
         */
        CounterExample(const CounterExample & ce) = default;
        /**
         * Move constructor.
         *
         * @param ce CounterExample to be moved.
         */
        CounterExample(CounterExample && ce);

        /**
         * Extracts constraints from the stored model concerning the input variables, i.e. extracts an expression
         * stating that the given input variables should have the values they have in the model.
         *
         * @param inputVars The variables for which the valuation is sought.
         * @param templ A pattern to relabel the variables concerning to the actual counter example number.
         * @return A constraint forcing relabeled inputVars to have the same valuation as they originally
         *         had in the stored model.
         */
        const z3::expr extractConstraints(const z3::expr_vector & inputVars,
                                                boost::format     templ,
                                                z3::context     * ctx) const;

        /**
         * Returns the serial number.
         *
         * @return The serial number.
         */
        const size_t getNumber() const;
    }; // CounterExample

    /**
     * Alias to use as findImplementation() return type.
     */
    using ImplementationTuple = std::tuple<boost::optional<Implementation>, z3::check_result>;
    /**
     * Alias to use as findCounterExample() return type.
     */
    using CounterExampleTuple = std::tuple<boost::optional<CounterExample>, z3::check_result>;

    /**
    * The result type of the CEGIS routine storing the found implementation, all the counter examples
    * needed and the runtime.
    */
    class CEGISResult {
        /**
         * Alias for the time_point type of the std::chrono library.
         */
        using TimePoint = std::chrono::high_resolution_clock::time_point;

    private:
        /**
         * Stores the found implementation. Contains none iff CEGIS routine failed.
         */
        const boost::optional<Implementation> implementation;
        /**
         * Solvers result.
         */
        const z3::check_result result;
        /**
         * Stores all the needed counter examples.
         */
        const std::vector<CounterExample> counterExamples;
        /**
         * Start and end point of the CEGIS routine.
         */
        const TimePoint startPoint, endPoint;
        /**
         * The name of the current implementation task.
         */
        const std::string name;

    public:
        /**
         * Constructor. Gets the implementation and the counter examples to store as well as two time points
         * representing the start and the end of the CEGIS routine.
         *
         * @param implTp The found valuation to the implementation variables plus the solver state.
         * @param ces The needed counter examples during the CEGIS routine.
         * @param start The time stamp where the CEGIS routine started.
         * @param end The time stamp where the CEGIS routine finished.
         * @param n The name of the current implementation task.
         */
        CEGISResult(const ImplementationTuple         & implTp,
                    const std::vector<CounterExample> & ces,
                    const TimePoint                   & start,
                    const TimePoint                   & end,
                    const std::string                 & n);

        /**
         * Indicates whether the CEGIS routine was able to find an implementation.
         *
         * @return z3::sat if an implementation was found,
         *         z3::unsat if no implementation is possible,
         *         z3::unknown if the solvers were not able to reason about it.
         */
        const z3::check_result check() const;

        /**
         * Returns the valuation to the given variable. Returns the variable itself iff no valuation exists.
         *
         * @return The valuation to the given (implementation) variables.
         */
        const z3::expr getValuation(const z3::expr var) const;

        /**
         * Gets the number of counter examples needed.
         *
         * @return The number of all needed counter examples.
         */
        const size_t getNumberOfCounterExamples() const;

        /**
         * Gets the runtime in milliseconds needed to execute the whole CEGIS routine.
         *
         * @return The runtime in milliseconds needed to execute the CEGIS routine.
         */
        long getRuntime() const;

        /**
         * Writes formatted results to the given std::ostream object. std::cout is used if no std::ostream object
         * is specified. Setting the csv flag to true outputs a comma-separated string line (ideal for
         * benchmarking).
         *
         * @param out The std::ostream object to which the results should be written.
         * @param csv A flag to indicate whether the output should be formatted comma-seperated.
         */
        void printResults(std::ostream & out = std::cout,
                          bool           csv = false);
    }; // CEGISResult

    /**
     * Alias for high_resolution_clock type of the std::chrono library.
     */
    using clock = std::chrono::high_resolution_clock;

    /**
     * The variables appearing in the expression to solve.
     */
    const z3::expr_vector implementationVariables, inputVariables, helperVariables;
    /**
     * The expression to solve splitted in different parts.
     */
    const z3::expr implementationExpression, behavioralExpression, correctnessExpression;
    /**
     * The given context used for all variables.
     */
    z3::context * const context;
    /**
     * Two solvers are used alternately within the CEGIS routine. One to find valuations to the implementation
     * variables and one to find counter examples for the found valuation.
     */
    z3::solver implementationSolver, counterExampleSolver;
    /**
     * A vector storing all found counter examples.
     */
    std::vector<CounterExample> counterExamples;

    /**
     * Counts the number of created counter examples.
     */
    static size_t ceID;

    /**
     * Template used for substituting the input variables during the CEGIS routine.
     */
    const boost::format INPUT_VAR_TEMPLATE{"inp_%1%_%2%"};
    /**
     * Template used for substituting the helper variables during the CEGIS routine.
     */
    const boost::format HELPER_VAR_TEMPLATE{"hlp_%1%_%2%"};

    /**
     * Name of the current implementation task.
     */
    std::string name = "";

    /**
     * Taking into account all found counter examples, this function calls the implementationSolver to find
     * a new valuation to the implementation variables that could satisfy the overall instance.
     *
     * @return A boost::optional containing this valuation as an Implementation object iff one was found.
     */
    const ImplementationTuple findImplementation();

    /**
     * Given the current Implementation, this function calls the counterExampleSolver to find a new valuation
     * to the input variables that leads to a contradiction.
     *
     * @param impl Current implementation for which a counter example should be found.
     * @return A boost::optional containing this contradiction, i.e. the valuation of the input variables, as
     * a CounterExample object iff one was found.
     */
    const CounterExampleTuple findCounterExample(const Implementation & impl);

    /**
     * Returns a z3::expr where all variables of vars appearing in expr are replaced by the pattern templ using the
     * current number of counter examples.
     *
     * @param expr Expression to be substituted.
     * @param vars Variables to be substituted in expr.
     * @param templ Pattern to perform the substitution.
     * @return A substituted expression.
     */
    const z3::expr substituteVars(const z3::expr        & expr,
                                  const z3::expr_vector & vars,
                                        boost::format     templ) const;

public:
    /**
     * Constructor. Creates a CEGISHandler object that is able to perform the CEGIS routine.
     * Requires a lot of parameters to keep the class immutable.
     *
     * @param ctx Pointer to the used context for all variables.
     * @param implVars The variables representing the sought implementation.
     * @param inpVars The variables representing the possible inputs.
     * @param hlpVars The variables representing the helpers.
     * @param implExpr Constraints ensuring validity of the implementation.
     * @param behavExpr Constraints ensuring correct internal behavior.
     * @param corrExpr Constraints ensuring desired output or similar.
     *
     */
    CEGISHandler(z3::context   * const   ctx,
                 z3::expr_vector const & implVars,
                 z3::expr_vector const & inpVars,
                 z3::expr_vector const & hlpVars,
                 z3::expr        const & implExpr,
                 z3::expr        const & behavExpr,
                 z3::expr        const & corrExpr);

    /**
     * Startes the CEGIS routine which tries to find a valuation to the input variables incrementally
     * by alternately calling two different solvers.
     *
     * @return a CEGISResult that stores the found implementation, all needed counter examples and the runtime.
     */
    const CEGISResult CEGISRoutine();

    /**
     * Sets the name of the current implementation task.
     *
     * @param n The desired name.
     */
    void setName(std::string n);

};

#endif //CEGIS_CEGIS_H
