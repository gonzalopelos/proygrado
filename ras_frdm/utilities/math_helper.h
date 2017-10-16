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

		/**
		 * returns absolute value of number.
		 */
		static float abs(float number){
			float result = number;
			if(number < 0){
				result = -result;
			}
			return result;
		}
		/**
		 * returns the difference (>= 0) between value1 and value2.
		 */
		static float diff(float value1, float value2){
			float result = -1;
			if(value1 > value2){
				result = value1 - value2;
			}else{
				result = value2 - value1;
			}

			return result;
		}
	};
}


#endif /* UTILITIES_MATH_HELPER_H_ */
