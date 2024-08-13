//*****************************************************************************
// NAME:
//      pubMLRegs.h
//
// DESCRIPTION:
//      Register definitions for MicroLoop-specific items
//
// AUTHORS:
//       Cody Burrows, AJ Frank
//
// CREATION DATE:
//      Mar 30, 2021
//
// COPYRIGHT NOTICE:
//        (C)Copyright 2015-2021  Teknic, Inc.  All rights reserved.
//
//        This copyright notice must be reproduced in any copy, modification,
//        or portion thereof merged into another program. A copy of the
//        copyright notice must be included in the object library of a user
//        program.
//                                                                            *
//*****************************************************************************
#ifndef _PUBMLREGS_H_
#define _PUBMLREGS_H_

//*****************************************************************************
// NAME                                                                       *
//     pubMLRegs.h headers
//
    #include "tekTypes.h"
    #include "underglowLEDctrl.h"
//                                                                            *
//*****************************************************************************

//*****************************************************************************
// NAME                                                                       *
//     pubMLRegs.h constants
//
    // Persist the connected motor type and connected phases, reset any errors
    #define REG_CLEAR_MASK  0x0000FE00
    // Blink Code Error groups
    #define GROUP_TWO       2
    #define GROUP_THREE     3
    #define GROUP_FOUR      4
    #define GROUP_FIVE      5
//                                                                            *
//*****************************************************************************

//*****************************************************************************
// NAME                                                                       *
//      MLmtrConnectStatus class
//
// DESCRIPTION
//      MicroLoop specific status register, encapsulated by a class to allow for
//      callback functions to be used when certain bits are set.
//                                                                            *
//*****************************************************************************
class MLmtrConnectStatus {
private:
    //*****************************************************************************
    // NAME                                                                       *
    //      MLflds struct
    //
    // DESCRIPTION
    //      MicroLoop specific status register to contain information about the
    //      currently connected motor. This is meant to provide information about
    //      the electrical connections like motor phases and the encoder.
    //                                                                            *
    //*****************************************************************************
    typedef struct _MLflds {
        /*
         * Encoder moves +/- one count when significant motion is expected
         */
        unsigned encCountErr : 1;           // 0
        /*
         * Feedback reports that motor is rotating backwards
         */
        unsigned backwards : 1;             // 1
        /*
         * Detected encoder resolution does not match what is defined in parameters
         */
        unsigned encRes : 1;                // 2
        /*
         * 5V encoder power supply overload
         */
        unsigned overload5V : 1;            // 3
        /*
         * Motor cable disconnected - detected by no load on the 5V encoder supply
         */
        unsigned mtrDisconnected : 1;       // 4

        unsigned rsvd0 : 3;                 // 5-7
        /*
         * Generic Motor Phase Error flag, set by an incorrect motor type or a
         * wiring issue.
         */
        unsigned mtrPhsErr : 1;             // 8
        /*
         * Displays the connected motor type. 1 = 3-phase, 2 = stepper, 4 = brush.
         */
        unsigned connectedMtrType : 3;      // 9-11
        /*
         * Displays what phases are detected as connected to the MicroLoop, each bit
         * is allocated for one phase, with the following order: 1 1 1 1
         *                                               phases: R S T U
         * A combination of these phases can be seen in hex, where if you have
         * phase U connected you'll see hex 8, and if you have all four phases
         * connected, you'll see hex F.
         */
        unsigned phasesConnected : 4;       // 12-15
        /*
         * The correct motor type is connected, but it's wired incorrectly.
         * For example, a stepper has phases wired R->S and T->U, or a
         * three-phase motor is connected to phases S, T, and U.
         */
        unsigned wiringErr : 1;            // 16
        /*
         * Detected phase resistance is different from what is stored in the
         * MtrOhms parameter.
         */
        unsigned phaseOhmsErr : 1;          // 17

        unsigned rsvd1 : 2;                 // 18-19
        /*
         * This motor has not yet been commissioned.
         */
        unsigned mtrNotCommission : 1;      // 20
        /*
         * Motor commissioning RO does not match detected motor RO.
         */
        unsigned mtrROmismatch : 1;         // 21

        unsigned rsvd2 : 10;                // 22-31
    } MLflds;

    //*****************************************************************************
    // NAME                                                                       *
    //      MLmtrConnectStatusReg union
    //
    // DESCRIPTION
    //      MicroLoop specific container to contain a copy of the MicroLoop
    //      Connected Motor Status Register. The contents can be accessed via \a
    //      bits field or via the individual fields via the \a fld field.
    //                                                                            *
    //*****************************************************************************
    typedef union MN_EXPORT PACK_BYTES _MLmtrConnectStatusReg {
        Uint32 bits;
        MLflds fld;
    } MLmtrConnectStatusReg;

    // Param Table access
    MLmtrConnectStatusReg m_register;

    // Flag for errors that require a Shutdown
    tBOOL m_error;

    // Use the underglow LEDs to display error codes
    UnderglowLEDcontrol m_underglow;
public:
    MLmtrConnectStatus() {
        // TODO: instantiate bits
        m_error = FALSE;
        m_underglow.Disable();
    }

    //Update the register and underglow LEDs
    void Update() {
        m_underglow.Update();
    }

    // Turn off the underglow, and reset any errors.
    void Clear(tBOOL saveMotorType) {
        if (saveMotorType) {
            m_register.bits &= REG_CLEAR_MASK;
        }
        else {
            m_register.bits = 0;
        }
        m_error = FALSE;
        SetUnderglowInactive();
    }

    // Does an error exist that should shut down the drive?
    tBOOL ErrorExists() {
        return m_error;
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Register accessor / setter functions
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // Get the value of the encCountErr bit
    unsigned encCountError() {
        return m_register.fld.encCountErr;
    }

    // Set the value of the encCountErr bit
    void encCountError(unsigned value) {
        m_register.fld.encCountErr = value;
        if (value) {
            m_error = TRUE;
            DisplayEncoderCountError();
        }
    }

    // Get the value of the backwards bit
    unsigned backwards() {
        return m_register.fld.backwards;
    }

    // Set the value of the backwards bit
    void backwards(unsigned value) {
        m_register.fld.backwards = value;
        if (value) {
            m_error = TRUE;
            DisplayBackwardsRotation();
        }
    }

    // Get the value of the encRes bit
    unsigned encRes() {
        return m_register.fld.encRes;
    }

    // Set the value of the encRes bit
    void encRes(unsigned value) {
        m_register.fld.encRes = value;
        if (value) {
            m_error = TRUE;
            DisplayEncoderResolutionMismatch();
        }
    }

    // Get the value of the overload5v bit
    unsigned overload5V() {
        return m_register.fld.overload5V;
    }

    // Set the value of the overload5v bit
    void overload5V(unsigned value) {
        m_register.fld.overload5V = value;
        if (value) {
            m_error = TRUE;
            DisplayEncoderPowerOverload();
        }
    }

    // Get the value of the mtrPhsErr bit
    unsigned mtrPhsErr() {
        return m_register.fld.mtrPhsErr;
    }

    // Set the value of the mtrPhsErr bit
    void mtrPhsErr(unsigned value) {
        m_register.fld.mtrPhsErr = value;
        if (value) {
            m_error = TRUE;
            DisplayMotorPhaseError();
        }
    }

    // Get the value of the connectedMtrType bits
    unsigned connectedMtrType() {
        // Right shift by one to return the motor type corresponding
        // to the motorTypes enum
        return (m_register.fld.connectedMtrType >> 1);
    }

    // Set the value of the connectedMtrType bits
    void connectedMtrType(unsigned value) {
        // Left shift 1 by the value, so that value: 0 = 1,
        // value: 1 = 2, value: 2 = 4, ...
        m_register.fld.connectedMtrType = (1 << value);
    }

    // Get the value of the phasesConnected bits
    unsigned phasesConnected() {
        return m_register.fld.phasesConnected;
    }

    // Set the value of the phasesConnected bits
    void phasesConnected(unsigned value) {
        m_register.fld.phasesConnected = value;
    }

    // Get the value of the wiringErr bit
    unsigned wiringErr() {
        return m_register.fld.wiringErr;
    }

    // Set the value of the wiringErr bit
    void wiringErr(unsigned value) {
        m_register.fld.wiringErr = value;
        if (value) {
            m_error = TRUE;
            DisplayMotorPhaseError();
        }
    }

    // Get the value of the phaseOhmsErr bit
    unsigned phaseOhmsErr() {
        return m_register.fld.phaseOhmsErr;
    }

    // Set the value of the phaseOhmsErr bit
    void phaseOhmsErr(unsigned value) {
        m_register.fld.phaseOhmsErr = value;
        if (value) {
            m_error = TRUE;
            DisplayPhaseResistanceIncorrect();
        }
    }

    // Get the value of the mtrNotCommission bit
    unsigned mtrNotCommission() {
        return m_register.fld.mtrNotCommission;
    }

    // Set the value of the mtrNotCommission bit
    void mtrNotCommission(unsigned value) {
        m_register.fld.mtrNotCommission = value;
        if (value) {
            m_error = TRUE;
            DisplayMotorNotCommissioned();
        }
    }

    // Get the value of the mtrNotConnected bit
    unsigned mtrNotConnected() {
        return m_register.fld.mtrDisconnected;
    }

    // Set the value of the mtrNotCommission bit
    void mtrNotConnected(unsigned value) {
        m_register.fld.mtrDisconnected = value;
        if (value) {
            m_error = TRUE;
            // Clear other bits to be re-evaluated once connected
            m_register.bits = 0x10L;
            DisplayMotorNotConnected();
        }
    }

    // Get the value of the mtrROmismatch bit
    unsigned mtrROmismatch() {
        return m_register.fld.mtrROmismatch;
    }

    // Set the value of the mtrROmismatch bit
    void mtrROmismatch(unsigned value) {
        m_register.fld.mtrROmismatch = value;
        if (value) {
            m_error = TRUE;
            DisplayMotorROmismatch();
        }
    }
private:

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Callback function definitions (intended to display error codes)
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // Encoder count error falls under Error Group 3.
    void DisplayEncoderCountError() {
        SetUnderglowActive(GROUP_THREE);
    }

    // Backwards motion error falls under Error Group 3.
    void DisplayBackwardsRotation() {
        SetUnderglowActive(GROUP_THREE);
    }

    // Encoder resolution error falls under Error Group 3.
    void DisplayEncoderResolutionMismatch() {
        SetUnderglowActive(GROUP_THREE);
    }

    // Encoder power overload error falls under Error Group 4.
    void DisplayEncoderPowerOverload() {
        SetUnderglowActive(GROUP_FOUR);
    }

    // Motor phases error falls under Error Group 1.
    void DisplayMotorPhaseError() {
        SetUnderglowActive(GROUP_TWO);
    }

    // Phase resistance error does not currently fall under an Error Group.
    void DisplayPhaseResistanceIncorrect() {

    }

    // Motor not commissioned error falls under Error Group 5.
    void DisplayMotorNotCommissioned() {
        SetUnderglowActive(GROUP_FIVE);
    }

    // Motor not connected error falls under Error Group 4.
    void DisplayMotorNotConnected() {
        SetUnderglowActive(GROUP_FOUR);
    }

    // RO mismatch error falls under Error Group 5.
    void DisplayMotorROmismatch() {
        SetUnderglowActive(GROUP_FIVE);
    }

    // Set the correct number of blinks, and activate the underglow.
    void SetUnderglowActive(int blinkCount) {
        m_underglow.SetBlinkCount(blinkCount);
        m_underglow.Blink();
    }

    // Disable the underglow.
    void SetUnderglowInactive() {
        // TODO: Clear underglow when clearing exceptions
        m_underglow.SetBlinkCount(0);
        m_underglow.Disable();
    }
};

#endif /* _PUBMLREGS_H_ */
