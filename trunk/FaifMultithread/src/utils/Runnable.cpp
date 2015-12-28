/*
 * Runnable.cpp
 *
 *  Created on: 1 Apr 2011
 *      Author: Maciej
 */

#include "Runnable.hpp"

Runnable::Runnable() {
	// TODO Auto-generated constructor stub
}

Runnable::~Runnable() {
	thrd->join();
}

void Runnable::join() {
	thrd->join();
}

void Runnable::start()
{
	thrd.reset(new boost::thread(boost::bind(&Runnable::run,this)));
}

void Runnable::sleep(int milis) {
	boost::xtime xt;
	boost::xtime_get(&xt, boost::TIME_UTC);
	xt.nsec += milis * 1000000;
	thrd->sleep(xt);
}


