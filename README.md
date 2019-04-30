# Shasta long read assembler
___

**The complete user documentation is available [here](https://chanzuckerberg.github.io/shasta/).**

**For quick start information see [here](https://chanzuckerberg.github.io/shasta/QuickStart.html).**
___

The goal of the Shasta long read assembler is to rapidly 
produce accurate assembled sequence using as input DNA reads
generated by [Oxford Nanopore](https://nanoporetech.com) flow cells.

Computational methods used by the Shasta assembler include:

* Using a
[run-length](https://en.wikipedia.org/wiki/Run-length_encoding)
representation of the read sequence.
This makes the assembly process more resilient to errors in
homopolymer repeat counts, which are the most common type
of errors in Oxford Nanopore reads. 

* Using in some phases of the computation a representation
of the read sequence based on *markers*, a fixed
subset of short k-mers (k ≈ 10).

An initial implementation of the Shasta assembler is complete and functional,
but significant improvements in several areas are possible.
As currently implemented, it can run an assembly of a human genome at coverage around 60x
in about 5 hours using a single, large machine (AWS instance type
`x1.32xlarge`, with 128 virtual processors and 1952 GB of memory).
The compute cost of such an assembly is around $20 at AWS spot market or reserved prices.

The accuracy of assembled sequence is being
analyzed. Early indications are that Shasta is similar or better 
in assembly quality when compared to other long read assemblers. 




#### Acknowledgments

The Shasta software uses various external software packages.
See [here](https://chanzuckerberg.github.io/shasta/Acknowledgments.html) for more information.

#### Reporting Security Issues
Please note: If you believe you have found a security issue, please responsibly disclose by contacting security@chanzuckerberg.com.
___

**The complete user documentation is available [here](https://chanzuckerberg.github.io/shasta/).**

**For quick start information see [here](https://chanzuckerberg.github.io/shasta/QuickStart.html).**
___




