//==========================================================================
// Name:            dlg_options.h
// Purpose:         Subclasses dialog GUI for general program options.
//                  (DlgOptions from topFrame.h)
// Created:         May. 11, 2012
// Initial author:  David Witten
// License:         BSD License (other licenses may apply to other
//                  components of this project)
//==========================================================================
#ifndef __COMPORTS_DIALOG__
#define __COMPORTS_DIALOG__

#include "fdmdv2_main.h"

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=
// Class ComPortsDlg
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--=-=-=-=
class ComPortsDlg : public DlgComPorts
{
    public:
        /** Constructor */
        ComPortsDlg( wxWindow* parent );
        void ExchangeData(int inout);

    protected:
        // Handlers for events.
        void OnListItemSelected( wxListEvent& event );
        void OnCancel( wxCommandEvent& event );
        void OnOK( wxCommandEvent& event );
        void OnClose( wxCloseEvent& event );
        void OnInitDialog( wxInitDialogEvent& event );
        void OnApply( wxCommandEvent& event );

};

#endif // __COMPORTS_DIALOG__
