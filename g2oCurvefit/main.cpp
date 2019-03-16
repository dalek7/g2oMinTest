// Simple curve-fitting example
// based on 
// https://github.com/RainerKuemmerle/g2o/blob/master/g2o/examples/data_fitting/curve_fit.cpp
// Tested on
// - Ubuntu 14.04 / 16.10
// - OS X (10.9.5)
//   : brew install homebrew/science/g2o
// - Windows 10 (VS2015)
// by Seung-Chan


#include <Eigen/Core>
#include <iostream>
#include "curve.h"

#include "g2o/stuff/sampler.h"
//#include "g2o/stuff/command_args.h"
//#include "g2o/core/solver.h"
#include "g2o/core/optimization_algorithm_levenberg.h"
#include "g2o/core/sparse_optimizer.h"
#include "g2o/solvers/linear_solver_dense.h"
#include "g2o/core/block_solver.h"

using namespace std;


int main(int argc, char** argv)
{
    cout << "Hello world!" << endl;

    int numPoints = 50;
    int maxIterations = 10;
    bool verbose = true;
    std::vector<int> gaugeList;
    string dumpFilename = "out_measurements.txt";
	/*
    g2o::CommandArgs arg;
    arg.param("dump", dumpFilename, "", "dump the points into a file");
    arg.param("numPoints", numPoints, 50, "number of points sampled from the curve");
    arg.param("i", maxIterations, 10, "perform n iterations");
    arg.param("v", verbose, false, "verbose output of the optimization process");
	
    arg.parseArgs(argc, argv);
	*/
    // generate random data
    double a = 2.;
    double b = 0.4;
    double lambda = 0.2;
	//a * exp(-lambda * t) + b

    Eigen::Vector2d* points = new Eigen::Vector2d[numPoints];
    for (int i = 0; i < numPoints; ++i)
    {
        double x = g2o::Sampler::uniformRand(0, 10);
        double y = a * exp(-lambda * x) + b;
        // add Gaussian noise
        y += g2o::Sampler::gaussRand(0, 0.02);
        points[i].x() = x;
        points[i].y() = y;
    }

    if (dumpFilename.size() < 1)
        dumpFilename = "out_measurements.txt";

    if(1)
    {
        ofstream fout(dumpFilename.c_str());
        for (int i = 0; i < numPoints; ++i)
          fout << points[i].transpose() << endl;
    }

    // some handy typedefs
    typedef g2o::BlockSolver< g2o::BlockSolverTraits<Eigen::Dynamic, Eigen::Dynamic> >  MyBlockSolver;
    typedef g2o::LinearSolverDense<MyBlockSolver::PoseMatrixType> MyLinearSolver;

    // setup the solver
    g2o::SparseOptimizer optimizer;
    optimizer.setVerbose(false);
    MyLinearSolver* linearSolver = new MyLinearSolver();
    MyBlockSolver* bsolver_ptr = new MyBlockSolver(linearSolver);
    g2o::OptimizationAlgorithmLevenberg* solver = new g2o::OptimizationAlgorithmLevenberg(bsolver_ptr);
    optimizer.setAlgorithm(solver);

    // build the optimization problem given the points
    // 1. add the parameter vertex
    VertexParams* params = new VertexParams();
    params->setId(0);
    params->setEstimate(Eigen::Vector3d(1,1,1)); // some initial value for the params
    optimizer.addVertex(params);


    // 2. add the points we measured to be on the curve
    MatrixXd m11 =Eigen::Matrix<double, 1, 1>::Identity();
    cout << m11 << endl;
    for (int i = 0; i < numPoints; ++i)
    {
        EdgePointOnCurve* e = new EdgePointOnCurve;
        e->setInformation(m11);
        e->setVertex(0, params);
        e->setMeasurement(points[i]);
        optimizer.addEdge(e);
    }

    // perform the optimization
    optimizer.initializeOptimization();
    optimizer.setVerbose(verbose);
    optimizer.optimize(maxIterations);

    //optimizer.save("out.g2o");
    if (verbose)
        cout << endl;

    // print out the result
    cout << "Target curve" << endl;
    cout << "a * exp(-lambda * x) + b" << endl;
    cout << "Iterative least squares solution" << endl;
    cout << "a      = " << params->estimate()(0) << endl;
    cout << "b      = " << params->estimate()(1) << endl;
    cout << "lambda = " << params->estimate()(2) << endl;
    cout << endl;

    ofstream fout2("out_params_est.txt");
    fout2 << params->estimate()(0) << endl;
    fout2 << params->estimate()(1) << endl;
    fout2 << params->estimate()(2) << endl;

    // clean up
    delete[] points;

	cin.ignore();
	//std::pause();
    return 0;
}

/* Output
(OS X)
 Target curve
 a * exp(-lambda * x) + b
 Iterative least squares solution
 a      = 1.99602
 b      = 0.403152
 lambda = 0.200758

 (Win10)
 a * exp(-lambda * x) + b
 Iterative least squares solution
 a      = 2.02449
 b      = 0.358788
 lambda = 0.19088
  */
