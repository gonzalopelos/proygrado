/*
 * Dm3Security.h
 *
 *  Created on: May 18, 2017
 *      Author: gonzalopelos
 */

#ifndef MODULES_DM3_DM3SECURITY_H_
#define MODULES_DM3_DM3SECURITY_H_

namespace modules {

class Dm3Security {
private:
	#define	ULTRASONIC_MIN_FRONT_DIST (int) 300
public:
	Dm3Security();
	virtual ~Dm3Security();
};

} /* namespace modules */

#endif /* MODULES_DM3_DM3SECURITY_H_ */
