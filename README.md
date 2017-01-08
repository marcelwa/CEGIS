# CEGIS

Counter-example guided inductive synthesis (CEGIS) implementation for the SMT solver
[Z3](https://github.com/Z3Prover/z3) by Microsoft Research. CEGIS is an approach to tackle
complexity of exact syntheses using satisfiability solvers. It was first introduced by
*Armando Solar-Lezama* in his Ph.D. Thesis
[Program Synthesis by Sketching](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.207.9048&rep=rep1&type=pdf)
back in 2008.

## What is CEGIS?

Synthesis tasks often have the same structure: an implementation is sought that behaves correctly under all possible
inputs (with the help of some extra variables, i.e. helper variables).

More formal: ∃ Implementation : ∀ Inputs : ∃ Helper Variables : Implementation behaves correctly.

It is absolutely legal to pass such a term to an SMT solver like Z3. A big drawback is the universal quantifier though.

For many real world problems it is not necessary to consider all inputs to derive an implementation that behaves
correctly for all of them. Following this observation, the problem was just moved to another position: which is the
minimal subset of inputs one have to consider to ensure a correct synthesis?

This is the point where CEGIS comes into play. CEGIS is a loop looking for exactly this minimal subset of inputs and
performing the implementation synthesis as a "by-product". Therefore CEGIS uses one satisfiability solver to generate
new implementations based on all the inputs considered so far (starting with zero); and another one to generate counter
examples that uncover incorrect behavior in the latest synthesised implementation. Eventually there will be no more
implementations possible, i.e. the specification is not realisable, or no more counter examples possible, i.e. the
latest implementation must be correct.

This CEGIS library works with the SMT solver Z3 and requires insight in the synthesis task to be executed, as it has to
be specified which variables belong to implementation, inputs, etc. Boundary conditions are to be specified manually as
well.

## Building and installing

Git, g++, cmake, a Python interpreter and the Boost Library are necessary in order to build CEGIS.
In *Ubuntu* the packages can be installed with

```sh
sudo apt-get install git g++ cmake python libboost-all-dev
```

Afterwards, CEGIS is ready to be installed.

```sh
git clone https://github.com/marcelwa/CEGIS.git
cd CEGIS
mkdir build
cd build
cmake ..
make
sudo make install
```

Since Z3 is automatically downloaded, configured and installed during this procedure, this step may take a while.
Please enter your super user password to provide installation rights.

The standard install prefix is `/usr/local/`. If another prefix is desired, just replace
`cmake ..` by `cmake -DCMAKE_INSTALL_PREFIX:PATH=/YOUR_PREFIX ..`. If you do not want Z3 to be downloaded and installed
during the build step because you have it already set up in your system, simply add `-DINSTALL_Z3=OFF` to your
`cmake` parameter list. Both options can be configured using `ccmake`, too,  of course.

### Uninstall

If you want to uninstall the CEGIS library and the downloaded Z3 solver, simply type

```sh
sudo xargs rm < install_manifest.txt
```

in the build directory.

## Usage

The functionality of this library is encapsulated in a class called `CEGISHandler`. This class handles the given
variables, constraints and execution of the CEGIS loop as well as runtime measurement and many other tasks.

```cpp
using namespace z3;

/* PREVIOUS USAGE */
context c;

// ...a lot of stuff...
 
solver s(c);
s.add(exists(implementation, forall(inputs, exists(helpers, constraints))));
s.check();


/* BETTER CEGIS USAGE */
context c;

// implementation, input and helper variables
expr_vector impl(c), inp(c), hlp(c);

// ...fill the vectors...

// implementation constraints, e.g. no cycles allowed
expr impl_cons(c);
// behavior constraints, e.g. desired propagation
expr behav_cons(c);
// correctness constraints, e.g. outputs depend on the inputs in a specifc way
expr corr_cons(c);

// ...fill all those...

// let's rock CEGIS
CEGISHandler handler(&c, impl, inp, hlp, impl_cons, behav_cons, corr_cons);
auto result = handler.CEGISRoutine();
result.print();
```
