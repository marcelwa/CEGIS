# CEGIS

Counter-example guided inductive synthesis (CEGIS) implementation for the SMT solver
[Z3](https://github.com/Z3Prover/z3) by Microsoft Research. CEGIS is an approach to tackle
complexity of exact syntheses using satisfiability solvers. It was first introduced by
*Armando Solar-Lezama* in his Ph.D. Thesis
[Program Synthesis by Sketching](http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.207.9048&rep=rep1&type=pdf)
back in 2008.

## Building

Obviously, Z3 is required to build the CEGIS library. In order to build Z3, Python
is needed first.

You can either use Python 2.x

```sh
sudo apt-get install python
```

or Python 3.x

```sh
sudo apt-get install python3
```

The latest version of Z3 can now be cloned and installed. This may take a while.

```sh
git clone https://github.com/Z3Prover/z3.git
cd z3
python3 scripts/mk_make.py --python
cd build
make
sudo make install
```

Replace `python3` by `python` if you chose installing Python 2.x.

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

The standard install prefix is `/usr/local`. If another prefix is desired, just replace `cmake ..` by `cmake -DCMAKE_INSTALL_PREFIX:PATH=/YOUR_PREFIX ..`
