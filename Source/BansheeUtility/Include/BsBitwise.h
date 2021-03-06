//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"

namespace bs 
{
	/** @addtogroup General
	 *  @{
	 */

	/** Floating point number broken down into components for easier access. */
	union Float754 
	{
		UINT32 raw;
		float value;
		struct {
#if BS_ENDIAN == BS_ENDIAN_BIG
			UINT32 negative : 1;
			UINT32 exponent : 8;
			UINT32 mantissa : 23;
#else
			UINT32 mantissa : 23;
			UINT32 exponent : 8;
			UINT32 negative : 1;
#endif
		} field;
	};

	/** 10-bit floating point number broken down into components for easier access. */
	union Float10
	{
		UINT32 raw;
		struct {
#if BS_ENDIAN == BS_ENDIAN_BIG
			UINT32 exponent : 5;
			UINT32 mantissa : 5;
#else
			UINT32 mantissa : 5;
			UINT32 exponent : 5;
#endif
		} field;
	};

	/** 11-bit floating point number broken down into components for easier access. */
	union Float11
	{
		UINT32 raw;
		struct {
#if BS_ENDIAN == BS_ENDIAN_BIG
			UINT32 exponent : 5;
			UINT32 mantissa : 6;
#else
			UINT32 mantissa : 6;
			UINT32 exponent : 5;
#endif
		} field;
	};

    /** Class for manipulating bit patterns. */
    class Bitwise 
	{
    public:
		/** Returns the most significant bit set in a value. */
		static UINT32 mostSignificantBitSet(unsigned int value)
        {
			UINT32 result = 0;
            while (value != 0) {
                ++result;
                value >>= 1;
            }
            return result-1;
        }

		/** Returns the power-of-two number greater or equal to the provided value. */
        static UINT32 nextPow2(UINT32 n)
        {
            --n;            
            n |= n >> 16;
            n |= n >> 8;
            n |= n >> 4;
            n |= n >> 2;
            n |= n >> 1;
            ++n;
            return n;
        }

		/** Returns the power-of-two number closest to the provided value. */
		static UINT32 closestPow2(UINT32 n)
		{
			UINT32 next = nextPow2(n);

			UINT32 prev = next >> 1;
			if (n - prev < next - n)
				return prev;
			
			return next;
		}

		/** Determines whether the number is power-of-two or not. */
        template<typename T>
        static bool isPow2(T n)
        {
            return (n & (n-1)) == 0;
        }

		/** Returns the number of bits a pattern must be shifted right by to remove right-hand zeros. */
		template<typename T>
        static unsigned int getBitShift(T mask)
		{
			if (mask == 0)
				return 0;

			unsigned int result = 0;
			while ((mask & 1) == 0) {
				++result;
				mask >>= 1;
			}
			return result;
		}

		/** Takes a value with a given src bit mask, and produces another value with a desired bit mask. */
		template<typename SrcT, typename DestT>
        static DestT convertBitPattern(SrcT srcValue, SrcT srcBitMask, DestT destBitMask)
		{
			// Mask off irrelevant source value bits (if any)
			srcValue = srcValue & srcBitMask;

			// Shift source down to bottom of DWORD
			const unsigned int srcBitShift = getBitShift(srcBitMask);
			srcValue >>= srcBitShift;

			// Get max value possible in source from srcMask
			const SrcT srcMax = srcBitMask >> srcBitShift;

			// Get max available in dest
			const unsigned int destBitShift = getBitShift(destBitMask);
			const DestT destMax = destBitMask >> destBitShift;

			// Scale source value into destination, and shift back
			DestT destValue = (srcValue * destMax) / srcMax;
			return (destValue << destBitShift);
		}

		/** 
		 * Convert N bit colour channel value to P bits. It fills P bits with the bit pattern repeated. 
		 * (this is /((1<<n)-1) in fixed point).
		 */
        static unsigned int fixedToFixed(UINT32 value, unsigned int n, unsigned int p) 
        {
            if(n > p) 
            {
                // Less bits required than available; this is easy
                value >>= n-p;
            } 
            else if(n < p)
            {
                // More bits required than are there, do the fill
                // Use old fashioned division, probably better than a loop
                if(value == 0)
                        value = 0;
                else if(value == (static_cast<unsigned int>(1)<<n)-1)
                        value = (1<<p)-1;
                else    value = value*(1<<p)/((1<<n)-1);
            }
            return value;    
        }

		/** 
		 * Convert floating point color channel value between 0.0 and 1.0 (otherwise clamped) to integer of a certain 
		 * number of bits. Works for any value of bits between 0 and 31.
		 */
        static unsigned int floatToFixed(const float value, const unsigned int bits)
        {
            if(value <= 0.0f) return 0;
            else if (value >= 1.0f) return (1<<bits)-1;
            else return (unsigned int)(value * (1<<bits));     
        }

		/** Fixed point to float. */
        static float fixedToFloat(unsigned value, unsigned int bits)
        {
            return (float)value/(float)((1<<bits)-1);
        }

		/** Write a n*8 bits integer value to memory in native endian. */
        static void intWrite(void *dest, const int n, const unsigned int value)
        {
            switch(n) {
                case 1:
                    ((UINT8*)dest)[0] = (UINT8)value;
                    break;
                case 2:
                    ((UINT16*)dest)[0] = (UINT16)value;
                    break;
                case 3:
#if BS_ENDIAN == BS_ENDIAN_BIG      
                    ((UINT8*)dest)[0] = (UINT8)((value >> 16) & 0xFF);
                    ((UINT8*)dest)[1] = (UINT8)((value >> 8) & 0xFF);
                    ((UINT8*)dest)[2] = (UINT8)(value & 0xFF);
#else
                    ((UINT8*)dest)[2] = (UINT8)((value >> 16) & 0xFF);
                    ((UINT8*)dest)[1] = (UINT8)((value >> 8) & 0xFF);
                    ((UINT8*)dest)[0] = (UINT8)(value & 0xFF);
#endif
                    break;
                case 4:
                    ((UINT32*)dest)[0] = (UINT32)value;                
                    break;                
            }        
        }

		/** Read a n*8 bits integer value to memory in native endian. */
        static unsigned int intRead(const void *src, int n) {
            switch(n) {
                case 1:
                    return ((UINT8*)src)[0];
                case 2:
                    return ((UINT16*)src)[0];
                case 3:
#if BS_ENDIAN == BS_ENDIAN_BIG      
                    return ((UINT32)((UINT8*)src)[0]<<16)|
                            ((UINT32)((UINT8*)src)[1]<<8)|
                            ((UINT32)((UINT8*)src)[2]);
#else
                    return ((UINT32)((UINT8*)src)[0])|
                            ((UINT32)((UINT8*)src)[1]<<8)|
                            ((UINT32)((UINT8*)src)[2]<<16);
#endif
                case 4:
                    return ((UINT32*)src)[0];
            } 
            return 0; // ?
        }

		/** Convert a float32 to a float16 (NV_half_float). */
        static UINT16 floatToHalf(float i)
        {
            union { float f; UINT32 i; } v;
            v.f = i;
            return floatToHalfI(v.i);
        }

		/** Converts float in UINT32 format to a a half in UINT16 format. */
        static UINT16 floatToHalfI(UINT32 i)
        {
            int s =  (i >> 16) & 0x00008000;
            int e = ((i >> 23) & 0x000000ff) - (127 - 15);
            int m =   i        & 0x007fffff;
        
            if (e <= 0)
            {
                if (e < -10)
                {
                    return 0;
                }
                m = (m | 0x00800000) >> (1 - e);
        
                return static_cast<UINT16>(s | (m >> 13));
            }
            else if (e == 0xff - (127 - 15))
            {
                if (m == 0) // Inf
                {
                    return static_cast<UINT16>(s | 0x7c00);
                } 
                else    // NAN
                {
                    m >>= 13;
                    return static_cast<UINT16>(s | 0x7c00 | m | (m == 0));
                }
            }
            else
            {
                if (e > 30) // Overflow
                {
                    return static_cast<UINT16>(s | 0x7c00);
                }
        
                return static_cast<UINT16>(s | (e << 10) | (m >> 13));
            }
        }
        
		/** Convert a float16 (NV_half_float) to a float32. */
        static float halfToFloat(UINT16 y)
        {
            union { float f; UINT32 i; } v;
            v.i = halfToFloatI(y);
            return v.f;
        }

		/** Converts a half in UINT16 format to a float in UINT32 format. */
        static UINT32 halfToFloatI(UINT16 y)
        {
            int s = (y >> 15) & 0x00000001;
            int e = (y >> 10) & 0x0000001f;
            int m =  y        & 0x000003ff;
        
            if (e == 0)
            {
                if (m == 0) // Plus or minus zero
                {
                    return s << 31;
                }
                else // Denormalized number -- renormalize it
                {
                    while (!(m & 0x00000400))
                    {
                        m <<= 1;
                        e -=  1;
                    }
        
                    e += 1;
                    m &= ~0x00000400;
                }
            }
            else if (e == 31)
            {
                if (m == 0) // Inf
                {
                    return (s << 31) | 0x7f800000;
                }
                else // NaN
                {
                    return (s << 31) | 0x7f800000 | (m << 13);
                }
            }
        
            e = e + (127 - 15);
            m = m << 13;
        
            return (s << 31) | (e << 23) | m;
        }

		/** Converts a 32-bit float to a 10-bit float according to OpenGL packed_float extension. */
		static UINT32 floatToFloat10(float v)
		{
			Float754 f;
			f.value = v;

			if (f.field.exponent == 0xFF)
			{
				// NAN or INF
				if (f.field.mantissa > 0)
					return 0x3E0 | (((f.raw >> 18) | (f.raw >> 13) | (f.raw >> 3) | f.raw) & 0x1F);
				else if (f.field.negative)
					return 0; // Negative infinity clamped to 0
				else
					return 0x3E0; // Positive infinity

			}
			else if (f.field.negative)
				return 0; // Negative clamped to 0, no negatives allowed
			else if (f.raw > 0x477C0000)
				return 0x3DF; // Too large, clamp to max value
			else
			{
				UINT32 val;
				if (f.raw < 0x38800000U)
				{
					// Too small to be represented as a normalized float, convert to denormalized value
					UINT32 shift = 113 - f.field.exponent;
					val = (0x800000U | f.field.mantissa) >> shift;
				}
				else
				{
					// Rebias exponent
					val = f.raw + 0xC8000000;
				}

				return  ((val + 0x1FFFFU + ((val >> 18) & 1)) >> 18) & 0x3FF;
			}
		}

		/** Converts a 32-bit float to a 11-bit float according to OpenGL packed_float extension. */
		static UINT32 floatToFloat11(float v)
		{
			Float754 f;
			f.value = v;

			if (f.field.exponent == 0xFF)
			{
				// NAN or INF
				if (f.field.mantissa > 0)
					return 0x7C0 | (((f.raw >> 17) | (f.raw >> 11) | (f.raw >> 6) | f.raw) & 0x3F);
				else if (f.field.negative)
					return 0; // Negative infinity clamped to 0
				else
					return 0x7C0; // Positive infinity

			}
			else if (f.field.negative)
				return 0; // Negative clamped to 0, no negatives allowed
			else if (f.raw > 0x477E0000)
				return 0x7BF; // Too large, clamp to max value
			else
			{
				UINT32 val;
				if(f.raw < 0x38800000U)
				{
					// Too small to be represented as a normalized float, convert to denormalized value
					UINT32 shift = 113 - f.field.exponent;
					val = (0x800000U | f.field.mantissa) >> shift;
				}
				else
				{
					// Rebias exponent
					val = f.raw + 0xC8000000;
				}

				return  ((val + 0xFFFFU + ((val >> 17) & 1)) >> 17) & 0x7FF;
			}
		}

		/** Converts a 10-bit float to a 32-bit float according to OpenGL packed_float extension. */
		static float float10ToFloat(UINT32 v)
		{
			Float10 f;
			f.raw = v;

			UINT32 output;
			if (f.field.exponent == 0x1F) // INF or NAN
			{
				output = 0x7f800000 | (f.field.mantissa << 17);
			}
			else
			{
				UINT32 exponent;
				UINT32 mantissa = f.field.mantissa;

				if (f.field.exponent != 0) // The value is normalized
					exponent = f.field.exponent;
				else if (mantissa != 0) // The value is denormalized
				{
					// Normalize the value in the resulting float
					exponent = 1;

					do
					{
						exponent--;
						mantissa <<= 1;
					} while ((mantissa & 0x20) == 0);

					mantissa &= 0x1F;
				}
				else // The value is zero
					exponent = (UINT32)-112;

				output = ((exponent + 112) << 23) | (mantissa << 18);
			}

			return *(float*)&output;
		}

		/** Converts a 11-bit float to a 32-bit float according to OpenGL packed_float extension. */
		static float float11ToFloat(UINT32 v)
		{
			Float11 f;
			f.raw = v;

			UINT32 output;
			if (f.field.exponent == 0x1F) // INF or NAN
			{
				output = 0x7f800000 | (f.field.mantissa << 17);
			}
			else
			{
				UINT32 exponent;
				UINT32 mantissa = f.field.mantissa;

				if (f.field.exponent != 0) // The value is normalized
					exponent = f.field.exponent;
				else if (mantissa != 0) // The value is denormalized
				{
					// Normalize the value in the resulting float
					exponent = 1;

					do
					{
						exponent--;
						mantissa <<= 1;
					} while ((mantissa & 0x40) == 0);

					mantissa &= 0x3F;
				}
				else // The value is zero
					exponent = (UINT32)-112;

				output = ((exponent + 112) << 23) | (mantissa << 17);
			}

			return *(float*)&output;
		}
    };

	/** @} */
}