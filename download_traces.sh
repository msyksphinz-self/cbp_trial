#!/bin/sh

mkdir -p traces
cd traces
curl -L http://hpca23.cse.tamu.edu/cbp2016/evaluationTraces.Final.tar | tar x
mv evaluationTraces/* .
