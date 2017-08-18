/*
 * math_helper.h
 *
 *  Created on: Aug 17, 2017
 *      Author: gonzalopelos
 */

#ifndef UTILITIES_MATH_HELPER_H_
#define UTILITIES_MATH_HELPER_H_


namespace utilities {
	class math_helper{
	public:
		static float abs(float number){
			float result = number;
			if(number < 0){
				result = -result;
			}
			return result;
		}
	};
}


#endif /* UTILITIES_MATH_HELPER_H_ */
