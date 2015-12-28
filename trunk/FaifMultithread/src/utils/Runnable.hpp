/*
 * Runnable.h
 *
 *  Created on: 1 Apr 2011
 *      Author: Maciej
 */

#ifndef RUNNABLE_H_
#define RUNNABLE_H_

#include <boost/bind.hpp>
#include <boost/thread/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include <memory>
#include <iostream>

class Runnable {
public:
	Runnable();
	virtual ~Runnable();
	virtual void run()=0;
	void start();
	void join();
	void sleep(int milis);
	boost::thread::id getId() {
		return thrd->get_id();
	}
private:
	Runnable(const Runnable&);
	boost::scoped_ptr<boost::thread> thrd;

};

#endif /* RUNNABLE_H_ */
