/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2010, Rice University
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of the Rice University nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Author: Ioan Sucan */


/* KAMIL:   This is a modified version of the Rigid Body Planning demo included with OMPL.  Use this to easily prototype
 *          new planners prior to including them in the OMPL build tree.
 *          NOTE: This requires a system-installed OMPL -- see CMakeLists.txt
 */

// Planner
#include "RRTsharp.h"

// OMPL
#include <ompl/config.h>
#include <ompl/base/SpaceInformation.h>
#include <ompl/base/spaces/SE3StateSpace.h>
#include <ompl/geometric/SimpleSetup.h>

// Standard Library
#include <iostream>
#include <fstream>
#include <string>
#include <functional>

namespace ob = ompl::base;
namespace og = ompl::geometric;

bool isStateValid(const ob::State *state)
{
    // cast the abstract state type to the type we expect
    const ob::SE3StateSpace::StateType *se3state = state->as<ob::SE3StateSpace::StateType>();

    // extract the first component of the state and cast it to what we expect
    const ob::RealVectorStateSpace::StateType *pos = se3state->as<ob::RealVectorStateSpace::StateType>(0);

    // extract the second component of the state and cast it to what we expect
    const ob::SO3StateSpace::StateType *rot = se3state->as<ob::SO3StateSpace::StateType>(1);

    // check validity of state defined by pos & rot


    // return a value that is always true but uses the two variables we define, so we avoid compiler warnings
    return (const void*)rot != (const void*)pos;
}

void plan(std::ofstream &out)
{
    // construct the state space we are planning in
    ob::StateSpacePtr space(new ob::SE3StateSpace());

    // set the bounds for the R^3 part of SE(3)
    ob::RealVectorBounds bounds(3);
    bounds.setLow(-1);
    bounds.setHigh(1);

    space->as<ob::SE3StateSpace>()->setBounds(bounds);

    // construct an instance of  space information from this state space
    ob::SpaceInformationPtr si(new ob::SpaceInformation(space));

    // set state validity checking for this space
    si->setStateValidityChecker(std::bind(&isStateValid, std::placeholders::_1));

    // create a random start state
    ob::ScopedState<> start(space);
    start.random();

    // create a random goal state
    ob::ScopedState<> goal(space);
    goal.random();

    // create a problem instance
    ob::ProblemDefinitionPtr pdef(new ob::ProblemDefinition(si));

    // set the start and goal states
    pdef->setStartAndGoalStates(start, goal);

    // create a planner for the defined space
    ob::PlannerPtr planner(new ompl::RRTsharp(si));
    planner->params().setParam("variant", "2");

    // set the problem we are trying to solve for the planner
    planner->setProblemDefinition(pdef);

    // perform setup steps for the planner
    planner->setup();

    // print the settings for this space
    si->printSettings(std::cout);

    // print the problem settings
    pdef->print(std::cout);

    // attempt to solve the problem within one second of planning time
    ob::PlannerStatus solved = planner->solve(1.0);

    if (solved)
    {
        // get the goal representation from the problem definition (not the same as the goal state)
        // and inquire about the found path
        ob::PathPtr path = pdef->getSolutionPath();
        std::cout << "Found solution:" << std::endl;

        // print the path to screen
        path->print(std::cout);

        // print the path to file
        og::PathGeometric* pg = path->as<og::PathGeometric>();
        pg->printAsMatrix(out);
    }
    else
        std::cout << "No solution found" << std::endl;
}

void planWithSimpleSetup(void)
{
    // construct the state space we are planning in
    ob::StateSpacePtr space(new ob::SE3StateSpace());

    // set the bounds for the R^3 part of SE(3)
    ob::RealVectorBounds bounds(3);
    bounds.setLow(-1);
    bounds.setHigh(1);

    space->as<ob::SE3StateSpace>()->setBounds(bounds);

    // define a simple setup class
    og::SimpleSetup ss(space);

    // set state validity checking for this space
    ss.setStateValidityChecker(std::bind(&isStateValid, std::placeholders::_1));

    // create a random start state
    ob::ScopedState<> start(space);
    start.random();

    // create a random goal state
    ob::ScopedState<> goal(space);
    goal.random();

    // set the start and goal states
    ss.setStartAndGoalStates(start, goal);

    // this call is optional, but we put it in to get more output information
    ss.setup();
    ss.print();

    // attempt to solve the problem within one second of planning time
    ob::PlannerStatus solved = ss.solve(1.0);

    if (solved)
    {
        std::cout << "Found solution:" << std::endl;
        // print the path to screen
        ss.simplifySolution();
        ss.getSolutionPath().print(std::cout);
    }
    else
        std::cout << "No solution found" << std::endl;
}

int main(int argc, char **argv)
{
    std::string outputfile = "solution.txt";

    // The following line WILL OVERWRITE the output file if it exists
    // To append instead, change std::ios::out to std::ios::app
    std::ofstream outstream(outputfile.c_str(), std::ios::out);

    std::cout << "OMPL version: " << OMPL_VERSION << std::endl;

    plan(outstream);
    outstream.close();

    std::cout << std::endl << std::endl;

    planWithSimpleSetup();

    return 0;
}
