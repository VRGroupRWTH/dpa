#!/bin/sh

cd ..
mkdir benchmarks
mkdir benchmarks/load_balancing
mkdir benchmarks/parameter_space
mkdir benchmarks/parameter_space/dataset_complexity
mkdir benchmarks/parameter_space/dataset_size
mkdir benchmarks/parameter_space/seed_distribution
mkdir benchmarks/parameter_space/seed_size
mkdir benchmarks/strong_scaling
mkdir benchmarks/weak_scaling

cd config/load_balancing
mv *.csv ../../benchmarks/load_balancing

cd ../parameter_space/dataset_complexity
mv *.csv ../../../benchmarks/parameter_space/dataset_complexity

cd ../dataset_size
mv *.csv ../../../benchmarks/parameter_space/dataset_size

cd ../seed_distribution
mv *.csv ../../../benchmarks/parameter_space/seed_distribution

cd ../seed_size
mv *.csv ../../../benchmarks/parameter_space/seed_size

cd ../../strong_scaling
mv *.csv ../../benchmarks/strong_scaling

cd ../weak_scaling
mv *.csv ../../benchmarks/weak_scaling