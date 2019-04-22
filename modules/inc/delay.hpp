/** \file delay.hpp
 * \brief delay implementation.
 *
 *  A module implementing a simple testing/calibration helper program that serves as a template.
 */
#ifndef INC_Delay_HPP_
#define INC_Delay_HPP_

#include "user_interface.hpp"
#include <via_platform_binding.hpp>
#include <oscillators.hpp>

#ifdef BUILD_F373

/// Macro used to specify the number of samples to per DAC transfer.
#define DELAY_BUFFER_SIZE 1

/// Callback to link to the C code in the STM32 Touch Sense Library.
void delayTouchLink (void *);

/// Calibration/template module class.
/** A simple self calibration tool that doubles as an introductory template.*/
class ViaDelay : public ViaModule {

public:

	/// Calibration UI implementation.
	/**
	 * Override virtual methods in the UI drivers to implement the touch interface.
	 * I believe this should use template meta-programming instead of virtual functions because specific functionality is determined at compile time.
	 * One C++ trick at a time for now.
	 **/

	class ViaDelayUI: public ViaUI {

	public:

		/**
		 * Pointer to the outer class to allow access to data and methods.
		 * See constructor and outer class constructor for details.
		 */
		ViaDelay& this_module;


		//@{
		/// Functions to handle a tap event on a touch sensor or shift combo.
		void button1TapCallback(void) override;
		void button2TapCallback(void) override;
		void button3TapCallback(void) override;
		void button4TapCallback(void) override;
		void button5TapCallback(void) override;
		void button6TapCallback(void) override;

		void aux1TapCallback(void) override;
		void aux2TapCallback(void) override;
		void aux2AltTapCallback(void) override;
		void aux3TapCallback(void) override;
		void aux4TapCallback(void) override;
		//@}

		//@{
		/// Functions to handle a hold event on a touch sensor or shift combo.
		void button1HoldCallback(void) override;
		void button2HoldCallback(void) override;
		void button3HoldCallback(void) override;
		void button4HoldCallback(void) override;
		void button5HoldCallback(void) override;
		void button6HoldCallback(void) override;

		void aux1HoldCallback(void) override;
		void aux2HoldCallback(void) override;
		void aux2AltHoldCallback(void) override;
		void aux3HoldCallback(void) override;
		void aux4HoldCallback(void) override;
		//@}

		//@{
		/// A method to access the LED control functions of this_module in the UI driver.
		void uiSetLEDs(int) override;
		//@}

		//@{
		/// A method to recall the module's last state at power on or restore presets.
		void recallModuleState(void) override;
		//@}

		//@{
		/// Methods to handle entry transitions into the main module states.
		void defaultEnterMenuCallback(void) override;
		void newModeEnterMenuCallback(void) override;
		void newAuxModeEnterMenuCallback(void) override;
		void presetEnterMenuCallback(void) override;
		//@}

		//@{
		/// Methods to handle entry transitions into touch sensor menu states.
		void button1EnterMenuCallback(void) override;
		void button2EnterMenuCallback(void) override;
		void button3EnterMenuCallback(void) override;
		void button4EnterMenuCallback(void) override;
		void button5EnterMenuCallback(void) override;
		void button6EnterMenuCallback(void) override;

		void aux1EnterMenuCallback(void) override;
		void aux2EnterMenuCallback(void) override;
		void aux2AltEnterMenuCallback(void) override;
		void aux3EnterMenuCallback(void) override;
		void aux4EnterMenuCallback(void) override;
		//@}

		/// A method to handle any initialization that needs to be done after some or all of the outer class constructor.
		void initialize(void) override;

		/// A utility method to write the factory presets from memory.
		void writeStockPresets(void) override {}

		/// On construction, link the calibTouchLink callback to the STM32 touch sense library.
		ViaDelayUI(ViaDelay& x): this_module(x) {
			linkUI((void *) &delayTouchLink, (void *) this);
		}

		//@{
		/// Some firmware-specific data members used to register touch events on each sensor for testing.
		int32_t touch1OK = 0;
		int32_t touch2OK = 0;
		int32_t touch3OK = 0;
		int32_t touch4OK = 0;
		int32_t touch5OK = 0;
		int32_t touch6OK = 0;
		///

	};

	/// An instance of the UI implementation.
	ViaDelayUI delayUI;

	/// A member that the UI implementation can use to turn the module's runtime display off.
	int32_t runtimeDisplay = 1;

	/// A utility method to pass UI interaction events to the touch sense library.
	/// Called in the executable code in user_interrupts.c.
	/// For some reason I have it wrapped like this?
	/// Perhaps the C code in the hardware executable mangled the namespace? Gotta check.
	void ui_dispatch(int32_t sig) {
		this->delayUI.dispatch(sig);
	};


	/*
	 *
	 * Processing functions
	 *
	 */

	/// Fill the dac buffers with fixed outputs.
	void process(int32_t writePosition);

#define  DELAY_LENGTH 4096
	int32_t * delayLine;
	int32_t delayTime = 65536;
	int32_t delayTimeTarget = 0;
	int32_t delayWrite = 0;
	int32_t lastReadPosition = 0;
	int32_t lastSample = 0;
	int32_t feedbackModOn = 0;
	int32_t delayModOn = 1;


	int32_t upsampleCounter = 16;
	int32_t delayIncrement = 0;

	int32_t burstCounter = 0;
	int32_t lfsrState = 1;
	void advanceLFSR(void) {
		lfsrState ^= lfsrState << 13;
		lfsrState ^= lfsrState << 17;
		lfsrState ^= lfsrState << 5;
		lfsrState &= 4095;
	}

	/// Instance of the exponential converter class.
	ExpoConverter revExpo;

	//@{
	/// Event handlers calling the corresponding methods from the state machine.
	void mainRisingEdgeCallback(void) {

		burstCounter = delayTime >> 16;

	}
	void mainFallingEdgeCallback(void) {
	}
	void auxRisingEdgeCallback(void) {

	}
	void auxFallingEdgeCallback(void) {
	}
	void buttonPressedCallback(void) {
	}
	void buttonReleasedCallback(void) {}
	void ioProcessCallback(void) {}
	void halfTransferCallback(void) {
		process(0);
	}
	void transferCompleteCallback(void) {
		process(DELAY_BUFFER_SIZE);
	}
	void slowConversionCallback(void) {
		controls.updateSlowExtra();
		delayTimeTarget = revExpo.convert(4095 - __USAT(controls.cv1Value - 2048 + controls.knob1Value, 12));
		delayTimeTarget = fix16_mul(delayTimeTarget, 160000);
	}
	void auxTimer1InterruptCallback(void) {

	}
	void auxTimer2InterruptCallback(void) {

	}

	/// On construction, call subclass constructors and pass each a pointer to the module class.
	ViaDelay() : delayUI(*this) {

		/// Link the module GPIO registers.
		initializeAuxOutputs();

		/// Initialize the input stream buffers.
		inputs.init(DELAY_BUFFER_SIZE);
		/// Initialize the output stream buffers.
		outputs.init(DELAY_BUFFER_SIZE);
		/// Set the data members that will be used to determine DMA stream initialization in the hardware executable.
		outputBufferSize = DELAY_BUFFER_SIZE;
		inputBufferSize = 1;

		delayLine = (int32_t *) malloc(sizeof(int32_t) * DELAY_LENGTH);


		/// Call the UI initialization that needs to happen after outer class construction.
		delayUI.initialize();


	}

};

#endif

#endif /* INC_Calib_HPP_ */
