//==========================================================================
// Name:            fdmdv2_main.h
//
// Purpose:         Declares simple wxWidgets application with GUI.
// Created:         Apr. 9, 2012
// Initial author:  David Witten
// License:         BSD License (other licenses may apply to other
//                  components of this project)
//==========================================================================
#ifndef __FDMDV2_MAIN__
#define __FDMDV2_MAIN__

#include <wx/wx.h>

#include <wx/tglbtn.h>
#include <wx/app.h>
#include "wx/rawbmp.h"
#include "wx/file.h"
#include "wx/filename.h"
#include "wx/config.h"
#include <wx/fileconf.h>
#include "wx/graphics.h"
#include "wx/mstream.h"
#include "wx/wfstream.h"
#include "wx/quantize.h"
#include "wx/scopedptr.h"
#include "wx/stopwatch.h"
#include "wx/versioninfo.h"
#include <wx/sound.h>

#include "codec2.h"
#include "fdmdv.h"
#include "fifo.h"

#include "topFrame.h"
#include "dlg_about.h"
#include "dlg_audio.h"
#include "dlg_options.h"
#include "dlg_comports.h"
#include "fdmdv2_plot.h"
#include "fdmdv2_plot_scalar.h"
#include "fdmdv2_plot_scatter.h"
#include "fdmdv2_plot_waterfall.h"
#include "fdmdv2_plot_spectrum.h"
#include "fdmdv2_pa_wrapper.h"
#include "sndfile.h"
#include "portaudio.h"

#define _USE_TIMER          1
#define _DUMMY_DATA         1
//#define _AUDIO_PASSTHROUGH  1

enum {
        ID_START = wxID_HIGHEST,
        ID_TIMER_WATERFALL,
        ID_TIMER_SPECTRUM,
        ID_TIMER_SCATTER,
        ID_TIMER_SCALAR
     };

#define EXCHANGE_DATA_IN    0
#define EXCHANGE_DATA_OUT   1

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=
// Class MainApp
//
// @class $(Name)
// @author $(User)
// @date $(Date)
// @file $(CurrentFileName).$(CurrentFileExt)
// @brief
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=
class MainApp : public wxApp
{
    public:
        virtual bool        OnInit();
        virtual int         OnExit();

        wxString            m_strVendName;
        wxString            m_StrAppName;

//        wxRadioButton       m_radioBtnPortAudio;
//        wxRadioButton       m_radioBtnFileOnly;

        wxString            m_textNumChOut;
        wxString            m_textNumChIn;

        wxString            m_strRxInAudio;
        wxString            m_strRxOutAudio;
        wxString            m_textVoiceInput;
        wxString            m_textVoiceOutput;
        wxString            m_strSampleRate;
        wxString            m_strBitrate;

        wxString            m_strRigCtrlPort;
        wxString            m_strRigCtrlBaud;
        wxString            m_strRigCtrlDatabits;
        wxString            m_strRigCtrlStopbits;
        wxString            m_strRigCtrlParity;

        wxRect              m_rTopWindow;

        bool                loadConfig();
        bool                saveConfig();

    protected:
};

// declare global static function wxGetApp()
DECLARE_APP(MainApp)

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=
// paCallBackData
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=
typedef struct
{
    PlotSpectrum    *pSPPanel;
    PlotWaterfall   *pWFPanel;
//    float           *mag_dB;
    float           in48k[FDMDV_OS_TAPS + N48];
    float           in8k[MEM8 + N8];
    struct FIFO    *infifo;
    struct FIFO    *outfifo;
} paCallBackData;

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=
// Class MainFrame
//
// @class $(Name)
// @author $(User)
// @date $(Date)
// @file $(CurrentFileName).$(CurrentFileExt)
// @brief
//
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=
class MainFrame : public TopFrame
{
    public:
        MainFrame(wxWindow *parent);
        virtual ~MainFrame();

//        PlotPanel*              m_panelDefaultA;
        PlotSpectrum*           m_panelSpectrum;
        PlotWaterfall*          m_panelWaterfall;
        PlotScatter*            m_panelScatter;
        PlotScalar*             m_panelScalar;
        bool                    m_SquelchActive;
        bool                    m_RxRunning;
        bool                    m_TxRunning;
        PortAudioWrap           *m_rxPa;
        PortAudioWrap           *m_txPa;
        paCallBackData          *m_rxUserdata;
        paCallBackData          *m_txUserdata;
        PaDeviceIndex           m_rxDevIn;
        PaDeviceIndex           m_rxDevOut;
        PaDeviceIndex           m_txDevIn;
        PaDeviceIndex           m_txDevOut;
        PaError                 m_rxErr;
        PaError                 m_txErr;
        wxSound                 *m_sound;
#ifdef _USE_TIMER
        wxTimer                 m_plotTimer;
#endif

        static int rxCallback(
                                const void *inBuffer,
                                void *outBuffer,
                                unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo *outTime,
                                PaStreamCallbackFlags statusFlags,
                                void *userData
                             );

        static int txCallback(
                                const void *inBuffer,
                                void *outBuffer,
                                unsigned long framesPerBuffer,
                                const PaStreamCallbackTimeInfo *outTime,
                                PaStreamCallbackFlags statusFlags,
                                void *userData
                             );

        static void per_frame_rx_processing(
                                        short   output_buf[],  // output buf of decoded speech samples
                                        int     *n_output_buf, // how many samples currently in output_buf[]
                                        int     codec_bits[],  // current frame of bits for decoder
                                        short   input_buf[],   // input buf of modem samples input to demod
                                        int     *n_input_buf,  // how many samples currently in input_buf[]
                                        int     *nin,          // amount of samples demod needs for next call
                                        int     *state,        // used to collect codec_bits[] halves
                                        struct  CODEC2 *c2     // Codec 2 states
                                    );

    protected:
        // protected event handlers
        virtual void OnCloseFrame(wxCloseEvent& event);
        virtual void OnExitClick(wxCommandEvent& event);
        //void OnQuit(wxCommandEvent& event);

        void averageData(float mag_dB[]);
        void startTxStream();
        void startRxStream();
        void stopTxStream();
        void stopRxStream();
        void abortTxStream();
        void abortRxStream();

        void OnOpen( wxCommandEvent& event );
        void OnOpenUpdateUI( wxUpdateUIEvent& event );
        void OnSave( wxCommandEvent& event );
        void OnSaveUpdateUI( wxUpdateUIEvent& event );
        void OnClose( wxCommandEvent& event );
        void OnCloseUpdateUI( wxUpdateUIEvent& event );
        void OnExit( wxCommandEvent& event );
        void OnCopy( wxCommandEvent& event );
        void OnCopyUpdateUI( wxUpdateUIEvent& event );
        void OnCut( wxCommandEvent& event );
        void OnCutUpdateUI( wxUpdateUIEvent& event );
        void OnPaste( wxCommandEvent& event );
        void OnPasteUpdateUI( wxUpdateUIEvent& event );
        void OnToolsAudio( wxCommandEvent& event );
        void OnToolsAudioUI( wxUpdateUIEvent& event );
        void OnToolsComCfg( wxCommandEvent& event );
        void OnToolsComCfgUI( wxUpdateUIEvent& event );
        void OnToolsOptions( wxCommandEvent& event );
        void OnToolsOptionsUI( wxUpdateUIEvent& event );
        void OnCaptureRxStream( wxCommandEvent& event );
        void OnCaptureTxStream( wxCommandEvent& event );
        void OnPlayAudioFile( wxCommandEvent& event );
        void OnHelpCheckUpdates( wxCommandEvent& event );
        void OnHelpCheckUpdatesUI( wxUpdateUIEvent& event );
        void OnHelpAbout( wxCommandEvent& event );
        void OnCmdSliderScroll( wxScrollEvent& event );
        void OnSliderScrollBottom( wxScrollEvent& event );
        void OnCmdSliderScrollChanged( wxScrollEvent& event );
        void OnSliderScrollTop( wxScrollEvent& event );
        void OnCheckSQClick( wxCommandEvent& event );

        // Toggle Buttons
        void OnTogBtnSplitClick(wxCommandEvent& event);
        void OnTogBtnAnalogClick(wxCommandEvent& event);
        void OnTogBtnALCClick(wxCommandEvent& event);
        void OnTogBtnRxID( wxCommandEvent& event );
        void OnTogBtnTxID( wxCommandEvent& event );
        void OnTogBtnTXClick( wxCommandEvent& event );
        void OnTogBtnOnOff( wxCommandEvent& event );

        void OnTogBtnSplitClickUI(wxUpdateUIEvent& event);
        void OnTogBtnAnalogClickUI(wxUpdateUIEvent& event);
        void OnTogBtnALCClickUI(wxUpdateUIEvent& event);
        void OnTogBtnRxIDUI(wxUpdateUIEvent& event );
        void OnTogBtnTxIDUI(wxUpdateUIEvent& event );
        void OnTogBtnTXClickUI(wxUpdateUIEvent& event );
        void OnTogBtnOnOffUI(wxUpdateUIEvent& event );

        //System Events
        void OnPaint(wxPaintEvent& event);
        void OnClose( wxCloseEvent& event );
        void OnSize( wxSizeEvent& event );
        void OnUpdateUI( wxUpdateUIEvent& event );
        void OnDeleteConfig(wxCommandEvent&);
#ifdef _USE_TIMER
        void OnTimer(wxTimerEvent &evt);
#else
        void OnIdle(wxIdleEvent &evt);
#endif

        wxString LoadUserImage(wxImage& image);

    private:
        bool CreateSound(wxSound& snd) const;
        wxString    m_soundFile;
#ifdef __WXMSW__
//        wxString    m_soundRes;
#endif // __WXMSW__
        bool        m_useMemory;
        wxTextCtrl* m_tc;
        int         m_zoom;
};

#endif //__FDMDV2_MAIN__
