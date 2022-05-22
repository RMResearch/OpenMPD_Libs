#ifndef _GSPAT_SOLUTION
#define _GSPAT_SOLUTION
#include <GSPAT_Solver_Prerequisites.h>
namespace GSPAT {
	class _GSPAT_Export Solution {
	public:
		/**
			Computes the messages to be sent to the boards and stores them in "out_top" and "out_bottom".
			This is all you need to use, if you are using GS_PAT. 
		*/
		virtual void finalMessages(unsigned char** out_top, unsigned char** out_bottom)=0;
		
		/**
			Returns the final array describing the phases of each transducer.
			LEVITATION SIGNATURE HAS BEEN APPLIED. 
			This method is used for debugging purposes, and for backwards compatibility (e.g. clients that did not use the discretization in the GPU solver). 
		*/
		virtual float* finalArrayPhases()=0;
		/**
			Returns the final array describing the amplitudes of each transducer.
			This method is used for debugging purposes, and for backwards compatibility (e.g. clients that did not use the discretization in the GPU solver). 
		*/
		virtual float* finalArrayAmplitudes()=0;
		/**
			Returns the final array describing the complex field (Re/Im for each transducer).
			NO LEVITATION SIGNATURE APPLIED-> So it is just a focusing hologram. 
			This method is retained for debugging purposes and can be used to visualize the fields. 
		*/
		virtual float* finalHologramReIm()=0;

		//virtual ~Solution() { ; }
	protected:
		Solution() { ; }
	};
};
#endif
