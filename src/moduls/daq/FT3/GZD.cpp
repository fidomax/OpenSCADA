//OpenSCADA module DAQ.FT3 file: GZD.cpp
/***************************************************************************
 *   Copyright (C) 2011-2016 by Maxim Kochetkov                            *
 *   fido_max@inbox.ru                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; version 2 of the License.               *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <sys/times.h>

#include <tsys.h>

#include "mod_FT3.h"
#include "GZD.h"

using namespace FT3;

void KA_GZD::SKAZDchannel::UpdateTUParam(uint16_t ID, uint8_t cl)
{
    ui8w tmp[6][2];
    tmp[0][0].w = TUOpen.Get();
    tmp[0][1].w = TimeOpen.Get();
    tmp[1][0].w = TUClose.Get();
    tmp[1][1].w = TimeClose.Get();
    tmp[2][0].w = TUStop.Get();
    tmp[2][1].w = TimeStop.Get();
    tmp[3][0].w = TURemote.Get();
    tmp[3][1].w = TimeRemote.Get();
    tmp[4][0].w = TUManual.Get();
    tmp[4][1].w = TimeManual.Get();
    tmp[5][0].w = TUStopEx.Get();
    tmp[5][1].w = TimeStopEx.Get();

    if(valve_type == vt_5TU) {
	if(tmp[0][0].w != TUOpen.vl || tmp[0][1].w != TimeOpen.vl || tmp[1][0].w != TUClose.vl || tmp[1][1].w != TimeClose.vl || tmp[2][0].w != TUStop.vl
		|| tmp[2][1].w != TimeStop.vl || tmp[3][0].w != TURemote.vl || tmp[3][1].w != TimeRemote.vl || tmp[4][0].w != TUManual.vl
		|| tmp[4][1].w != TimeManual.vl) {
	    TUOpen.s = 0;
	    TUOpen.Update(tmp[0][0].w);
	    TimeOpen.Update(tmp[0][1].w);
	    TUClose.Update(tmp[1][0].w);
	    TimeClose.Update(tmp[1][1].w);
	    TUStop.Update(tmp[2][0].w);
	    TimeStop.Update(tmp[2][1].w);
	    TURemote.Update(tmp[3][0].w);
	    TimeRemote.Update(tmp[3][1].w);
	    TUManual.Update(tmp[4][0].w);
	    TimeManual.Update(tmp[4][1].w);
	    uint8_t E[21] = { 0, tmp[0][0].b[0], tmp[0][0].b[1], tmp[0][1].b[0], tmp[0][1].b[1], tmp[1][0].b[0], tmp[1][0].b[1], tmp[1][1].b[0], tmp[1][1].b[1],
		    tmp[2][0].b[0], tmp[2][0].b[1], tmp[2][1].b[0], tmp[2][1].b[1], tmp[3][0].b[0], tmp[3][0].b[1], tmp[3][1].b[0], tmp[3][1].b[1],
		    tmp[4][0].b[0], tmp[4][0].b[1], tmp[4][1].b[0], tmp[4][1].b[1] };
	    da->PushInBE(cl, sizeof(E), ID, E);
	}
    }
    if(valve_type == vt_6TU) {
	if(tmp[0][0].w != TUOpen.vl || tmp[0][1].w != TimeOpen.vl || tmp[1][0].w != TUClose.vl || tmp[1][1].w != TimeClose.vl || tmp[2][0].w != TUStop.vl
		|| tmp[2][1].w != TimeStop.vl || tmp[3][0].w != TURemote.vl || tmp[3][1].w != TimeRemote.vl || tmp[4][0].w != TUManual.vl
		|| tmp[4][1].w != TimeManual.vl || tmp[5][0].w != TUStopEx.vl || tmp[5][1].w != TimeStopEx.vl) {
	    TUOpen.s = 0;
	    TUOpen.Update(tmp[0][0].w);
	    TimeOpen.Update(tmp[0][1].w);
	    TUClose.Update(tmp[1][0].w);
	    TimeClose.Update(tmp[1][1].w);
	    TUStop.Update(tmp[2][0].w);
	    TimeStop.Update(tmp[2][1].w);
	    TURemote.Update(tmp[3][0].w);
	    TimeRemote.Update(tmp[3][1].w);
	    TUManual.Update(tmp[4][0].w);
	    TimeManual.Update(tmp[4][1].w);
	    TUStopEx.Update(tmp[5][0].w);
	    TimeStopEx.Update(tmp[5][1].w);
	    uint8_t E[25] = { 0, tmp[0][0].b[0], tmp[0][0].b[1], tmp[0][1].b[0], tmp[0][1].b[1], tmp[1][0].b[0], tmp[1][0].b[1], tmp[1][1].b[0], tmp[1][1].b[1],
		    tmp[2][0].b[0], tmp[2][0].b[1], tmp[2][1].b[0], tmp[2][1].b[1], tmp[3][0].b[0], tmp[3][0].b[1], tmp[3][1].b[0], tmp[3][1].b[1],
		    tmp[4][0].b[0], tmp[4][0].b[1], tmp[4][1].b[0], tmp[4][1].b[1], tmp[5][0].b[0], tmp[5][0].b[1], tmp[5][1].b[0], tmp[5][1].b[1] };
	    da->PushInBE(cl, sizeof(E), ID, E);
	}
    }

}

void KA_GZD::SKAZDchannel::UpdateTCParam(uint16_t ID, uint8_t cl)
{
    ui8w tmp[5];
    tmp[0].w = TCOpen.Get();
    tmp[1].w = TCClose.Get();
    tmp[2].w = TCMode.Get();
    tmp[3].w = TCOpenErr.Get();
    tmp[4].w = TCCloseErr.Get();

    if(tmp[0].w != TCOpen.vl || tmp[1].w != TCClose.vl || tmp[2].w != TCMode.vl || tmp[3].w != TCOpenErr.vl || tmp[4].w != TCCloseErr.vl) {
	TCOpen.s = 0;
	TCOpen.Update(tmp[0].w);
	TCClose.Update(tmp[1].w);
	TCMode.Update(tmp[2].w);
	TCOpenErr.Update(tmp[3].w);
	TCCloseErr.Update(tmp[4].w);
	uint8_t E[11] = { 0, tmp[0].b[0], tmp[0].b[1], tmp[1].b[0], tmp[1].b[1], tmp[2].b[0], tmp[2].b[1], tmp[3].b[0], tmp[3].b[1], tmp[4].b[0], tmp[4].b[1] };
	da->PushInBE(cl, sizeof(E), ID, E);
    }
}

uint8_t KA_GZD::SKAZDchannel::SetNewTUParam(uint8_t addr, uint16_t prmID, uint8_t *val)
{
    if(TUOpen.lnk.Connected() || TimeOpen.lnk.Connected() || TUClose.lnk.Connected() || TimeClose.lnk.Connected() || TUStop.lnk.Connected()
	    || TimeOpen.lnk.Connected() || TURemote.lnk.Connected() || TimeRemote.lnk.Connected() || TUManual.lnk.Connected() || TimeManual.lnk.Connected()) {
	if(valve_type == vt_6TU) {
	    if(TUStopEx.lnk.Connected() || TimeStopEx.lnk.Connected()) {
	    } else {
		return 0;
	    }
	}
	TUOpen.s = addr;
	TUOpen.Set(TSYS::getUnalign16(val));
	TimeOpen.Set(TSYS::getUnalign16(val + 2));
	TUClose.Set(TSYS::getUnalign16(val + 4));
	TimeClose.Set(TSYS::getUnalign16(val + 6));
	TUStop.Set(TSYS::getUnalign16(val + 8));
	TimeStop.Set(TSYS::getUnalign16(val + 10));
	TURemote.Set(TSYS::getUnalign16(val + 12));
	TimeRemote.Set(TSYS::getUnalign16(val + 14));
	TUManual.Set(TSYS::getUnalign16(val + 16));
	TimeManual.Set(TSYS::getUnalign16(val + 18));
	if(valve_type == vt_5TU) {
	    uint8_t E[21];
	    E[0] = addr;
	    memcpy(E + 1, val, 20);
	    da->PushInBE(1, sizeof(E), prmID, E);
	    return 2 + 20;
	}
	if(valve_type == vt_6TU) {
	    TUStop.Set(TSYS::getUnalign16(val + 20));
	    TimeStop.Set(TSYS::getUnalign16(val + 22));
	    uint8_t E[25];
	    E[0] = addr;
	    memcpy(E + 1, val, 24);
	    da->PushInBE(1, sizeof(E), prmID, E);
	    return 2 + 24;
	}

    } else {
	return 0;
    }
}

uint8_t KA_GZD::SKAZDchannel::SetNewTCParam(uint8_t addr, uint16_t prmID, uint8_t *val)
{
    if(TCOpen.lnk.Connected() || TCClose.lnk.Connected() || TCMode.lnk.Connected() || TCOpenErr.lnk.Connected() || TCCloseErr.lnk.Connected()) {
	TCOpen.s = addr;
	TCOpen.Set(TSYS::getUnalign16(val));
	TCClose.Set(TSYS::getUnalign16(val + 2));
	TCMode.Set(TSYS::getUnalign16(val + 4));
	TCOpenErr.Set(TSYS::getUnalign16(val + 6));
	TCCloseErr.Set(TSYS::getUnalign16(val + 8));

	uint8_t E[11];
	E[0] = addr;
	memcpy(E + 1, val, 10);
	da->PushInBE(1, sizeof(E), prmID, E);
	return 2 + 10;
    } else {
	return 0;
    }
}

uint8_t KA_GZD::SKAZDchannel::SetNewState(uint8_t addr, uint16_t prmID, uint8_t *val)
{
    uint8_t rc = 0;
    if(State.lnk.Connected()) {
	if(Function.vl == 0) {
	    State.s = addr;
	    State.Set((uint8_t) ((State.vl & 0x80) | *val));
	    uint8_t E[2] = { addr, State.vl };
	    da->PushInBE(1, sizeof(E), prmID, E);
	    rc = 2 + 1;
	}
    }
    return rc;
}

uint8_t KA_GZD::SKAZDchannel::SetNewFunction(uint8_t addr, uint16_t prmID, uint8_t *val)
{
    uint8_t rc = 0;
    uint32_t newF = *val;
    if(Function.lnk.Connected() && State.lnk.Connected()) {
	if(((State.vl & 0x0F) == vs_06) || Function.vl) {
	    if(Function.vl == newF) {
		rc = 3;
	    } else {
		if(newF > 2) {
		    Function.s = addr;
		    Function.Set(newF << 16 | newF);
		    State.Set((uint8_t) (State.vl & 0x8F));
		    uint8_t E[2] = { addr, Function.vl };
		    da->PushInBE(1, sizeof(E), prmID, E);
		    rc = 3;
		}
	    }
	} else {
	    Function.s = addr;
	    Function.Set(newF << 16 | newF);
	    State.Set((uint8_t) (State.vl & 0x8F));
	    uint8_t E[2] = { addr, Function.vl };
	    da->PushInBE(1, sizeof(E), prmID, E);
	    rc = 3;
	}
    }
    return rc;
}

KA_GZD::KA_GZD(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params, uint32_t v_type) :
	DA(prm), ID(id), count_n(n), with_params(has_params), valve_type(v_type), config(0xF | (n << 4) | (2 << 10))
{
    mTypeFT3 = KA;
    chan_err.clear();
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");

    for(int i = 0; i < count_n; i++) {
	chan_err.insert(chan_err.end(), SDataRec());
	AddZDChannel(i);
    }
    loadIO(true);
}

KA_GZD::~KA_GZD()
{

}

void KA_GZD::AddZDChannel(uint8_t iid)
{
    data.push_back(SKAZDchannel(iid, with_params, valve_type, this));
    AddAttr(data.back().State.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:0", iid + 1));
    AddAttr(data.back().Function.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:3", iid + 1));
    if(with_params) {
	AddAttr(data.back().TUOpen.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	AddAttr(data.back().TUClose.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	AddAttr(data.back().TUStop.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	if(valve_type == vt_6TU) {
	    AddAttr(data.back().TUStopEx.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	}
	AddAttr(data.back().TURemote.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	AddAttr(data.back().TUManual.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	AddAttr(data.back().TimeOpen.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	AddAttr(data.back().TimeClose.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	AddAttr(data.back().TimeStop.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	if(valve_type == vt_6TU) {
	    AddAttr(data.back().TimeStopEx.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	}
	AddAttr(data.back().TimeRemote.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	AddAttr(data.back().TimeManual.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	AddAttr(data.back().TCOpen.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().TCClose.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().TCMode.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().TCOpenErr.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().TCCloseErr.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
    }
}

string KA_GZD::getStatus(void)
{
    string rez;
    if(NeedInit) {
	rez = "20: ?????????? ??????????????:";
	for(int i = 1; i <= count_n; i++) {
	    switch(chan_err[i].state) {
	    case 0:
		rez += TSYS::strMess(" %d.", i);
		break;
	    case 2:
		rez += TSYS::strMess(" %d!!!", i);
		break;
	    }
	}
    } else {
	rez = "0: ??????????";
    }
    return rez;
}

uint16_t KA_GZD::GetState()
{
    tagMsg Msg;
    uint16_t rc = BlckStateUnknown;
    Msg.L = 5;
    Msg.C = AddrReq;
    *((uint16_t *) Msg.D) = PackID(ID, 0, 0); //state
    if(mPrm.owner().DoCmd(&Msg) == GOOD3) {
	switch(mPrm.vlAt("state").at().getI(0, true)) {
	case KA_GZD_Error:
	    rc = BlckStateError;
	    break;
	case KA_GZD_Normal:
	    rc = BlckStateNormal;
	    break;
	}
    }
    return rc;
}

uint16_t KA_GZD::SetParams(void)
{
    uint16_t rc;
    tagMsg Msg;
    loadParam();
    for(int i = 0; i < count_n; i++) {
	Msg.L = 0;
	Msg.C = SetData;
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 0));
	Msg.L += SerializeB(Msg.D + Msg.L, data[i].State.lnk.vlattr.at().getI(0, true) & 0x0F);
	if((data[i].State.lnk.vlattr.at().getI(0, true) & 0x0F) != vs_06) {
	    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 1));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TUOpen.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TimeOpen.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TUClose.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TimeClose.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TUStop.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TimeStop.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TURemote.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TimeRemote.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TUManual.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TimeManual.lnk.vlattr.at().getI(0, true));
	    if(valve_type == vt_6TU) {
		Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TUStopEx.lnk.vlattr.at().getI(0, true));
		Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TimeStopEx.lnk.vlattr.at().getI(0, true));
	    }
	    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 2));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TCOpen.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TCClose.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TCMode.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TCOpenErr.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, data[i].TCCloseErr.lnk.vlattr.at().getI(0, true));
	}
	Msg.L += 3;
	rc = mPrm.owner().DoCmd(&Msg);
	if((rc == BAD2) || (rc == BAD3)) {
	    mPrm.mess_sys(TMess::Error, "Can't set channel %d", i + 1);
	} else {
	    if(rc == ERROR) {
		mPrm.mess_sys(TMess::Error, "No answer to set channel %d", i + 1);
		break;
	    }
	}
    }
    return rc;
}

uint16_t KA_GZD::RefreshParams(void)
{
    uint16_t rc;
    tagMsg Msg;
    for(int i = 1; i <= count_n; i++) {
	Msg.L = 0;
	Msg.C = AddrReq;
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i, 1));
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i, 2));
	Msg.L += 3;
	rc = mPrm.owner().DoCmd(&Msg);
	if((rc == BAD2) || (rc == BAD3)) {
	    mPrm.mess_sys(TMess::Error, "Can't refresh channel %d params", i);
	} else {
	    if(rc == ERROR) {
		mPrm.mess_sys(TMess::Error, "No answer to refresh channel %d params", i);
		break;
	    }
	}
    }
    return rc;
}

uint16_t KA_GZD::RefreshData(void)
{
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = AddrReq;
    for(int i = 1; i <= count_n; i++) {
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i, 0));
    }
    Msg.L += 3;
    return mPrm.owner().DoCmd(&Msg);
}

void KA_GZD::loadIO(bool force)
{
    if(mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }
    for(int i = 0; i < count_n; i++) {
	loadLnk(data[i].State.lnk);
	loadLnk(data[i].Function.lnk);
	loadLnk(data[i].TUOpen.lnk);
	loadLnk(data[i].TUClose.lnk);
	loadLnk(data[i].TUStop.lnk);
	loadLnk(data[i].TUStopEx.lnk);
	loadLnk(data[i].TURemote.lnk);
	loadLnk(data[i].TUManual.lnk);
	loadLnk(data[i].TimeOpen.lnk);
	loadLnk(data[i].TimeClose.lnk);
	loadLnk(data[i].TimeStop.lnk);
	loadLnk(data[i].TimeStopEx.lnk);
	loadLnk(data[i].TimeRemote.lnk);
	loadLnk(data[i].TimeManual.lnk);
	loadLnk(data[i].TCOpen.lnk);
	loadLnk(data[i].TCClose.lnk);
	loadLnk(data[i].TCMode.lnk);
	loadLnk(data[i].TCOpenErr.lnk);
	loadLnk(data[i].TCCloseErr.lnk);
    }
}

void KA_GZD::saveIO(void)
{
    for(int i = 0; i < count_n; i++) {
	saveLnk(data[i].State.lnk);
	saveLnk(data[i].Function.lnk);
	saveLnk(data[i].TUOpen.lnk);
	saveLnk(data[i].TUClose.lnk);
	saveLnk(data[i].TUStop.lnk);
	saveLnk(data[i].TUStopEx.lnk);
	saveLnk(data[i].TURemote.lnk);
	saveLnk(data[i].TUManual.lnk);
	saveLnk(data[i].TimeOpen.lnk);
	saveLnk(data[i].TimeClose.lnk);
	saveLnk(data[i].TimeStop.lnk);
	saveLnk(data[i].TimeStopEx.lnk);
	saveLnk(data[i].TimeRemote.lnk);
	saveLnk(data[i].TimeManual.lnk);
	saveLnk(data[i].TCOpen.lnk);
	saveLnk(data[i].TCClose.lnk);
	saveLnk(data[i].TCMode.lnk);
	saveLnk(data[i].TCOpenErr.lnk);
	saveLnk(data[i].TCCloseErr.lnk);
    }
}

void KA_GZD::saveParam(void)
{
    for(int i = 0; i < count_n; i++) {
	saveVal(data[i].State.lnk);
	saveVal(data[i].TUOpen.lnk);
	saveVal(data[i].TUClose.lnk);
	saveVal(data[i].TUStop.lnk);
	saveVal(data[i].TURemote.lnk);
	saveVal(data[i].TUManual.lnk);
	saveVal(data[i].TimeOpen.lnk);
	saveVal(data[i].TimeClose.lnk);
	saveVal(data[i].TimeStop.lnk);
	saveVal(data[i].TimeRemote.lnk);
	saveVal(data[i].TimeManual.lnk);
	saveVal(data[i].TCOpen.lnk);
	saveVal(data[i].TCClose.lnk);
	saveVal(data[i].TCMode.lnk);
	saveVal(data[i].TCOpenErr.lnk);
	saveVal(data[i].TCCloseErr.lnk);
	if(valve_type == vt_6TU) {
	    saveVal(data[i].TUStopEx.lnk);
	    saveVal(data[i].TimeStopEx.lnk);
	}
    }
}

void KA_GZD::loadParam(void)
{
    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "load param");
    for(int i = 0; i < count_n; i++) {
	loadVal(data[i].State.lnk);
	loadVal(data[i].TUOpen.lnk);
	loadVal(data[i].TUClose.lnk);
	loadVal(data[i].TUStop.lnk);
	loadVal(data[i].TURemote.lnk);
	loadVal(data[i].TUManual.lnk);
	loadVal(data[i].TimeOpen.lnk);
	loadVal(data[i].TimeClose.lnk);
	loadVal(data[i].TimeStop.lnk);
	loadVal(data[i].TimeRemote.lnk);
	loadVal(data[i].TimeManual.lnk);
	loadVal(data[i].TCOpen.lnk);
	loadVal(data[i].TCClose.lnk);
	loadVal(data[i].TCMode.lnk);
	loadVal(data[i].TCOpenErr.lnk);
	loadVal(data[i].TCCloseErr.lnk);
	if(valve_type == vt_6TU) {
	    loadVal(data[i].TUStopEx.lnk);
	    loadVal(data[i].TimeStopEx.lnk);
	}
    }
}

void KA_GZD::tmHandler(void)
{
    for(int i = 0; i < count_n; i++) {
	if(with_params) {
	    data[i].UpdateTUParam(PackID(ID, (i + 1), 1), 1);
	    data[i].UpdateTCParam(PackID(ID, (i + 1), 2), 1);
	}
	UpdateParam8(data[i].State, PackID(ID, (i + 1), 0), 1);
	UpdateParam8(data[i].Function, PackID(ID, (i + 1), 3), 1);
    }
    NeedInit = false;
}

uint16_t KA_GZD::HandleEvent(int64_t tm, uint8_t * D)
{
    FT3ID ft3ID = UnpackID(TSYS::getUnalign16(D));
    if(ft3ID.g != ID) return 0;
    uint16_t l = 0;
    switch(ft3ID.k) {
    case 0:
	switch(ft3ID.n) {
	case 0:
	    mPrm.vlAt("state").at().setI(D[2], tm, true);
	    l = 3;
	    break;
	case 1:
	    l = 4;
	    break;
	case 2:
	    l = 2 + count_n * 2;
	    for(int j = 0; j < count_n; j++) {
		mPrm.vlAt(TSYS::strMess("state_%d", j)).at().setI(D[j * 2 + 3], tm, true);
	    }
	    break;
	}
	break;
    default:
	if(ft3ID.k && (ft3ID.k <= count_n)) {
	    switch(ft3ID.n) {
	    case 0:
		mPrm.vlAt(TSYS::strMess("state_%d", ft3ID.k)).at().setI(D[3], tm, true);
		l = 4;
		break;
	    case 1:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("TUopen_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 3), tm, true);
		    mPrm.vlAt(TSYS::strMess("timeOpen_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 5), tm, true);
		    mPrm.vlAt(TSYS::strMess("TUclose_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 7), tm, true);
		    mPrm.vlAt(TSYS::strMess("timeClose_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 9), tm, true);
		    mPrm.vlAt(TSYS::strMess("TUstop_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 11), tm, true);
		    mPrm.vlAt(TSYS::strMess("timeStop_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 13), tm, true);
		    mPrm.vlAt(TSYS::strMess("TUremote_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 15), tm, true);
		    mPrm.vlAt(TSYS::strMess("timeRemote_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 17), tm, true);
		    mPrm.vlAt(TSYS::strMess("TUmanual_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 19), tm, true);
		    mPrm.vlAt(TSYS::strMess("timeManual_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 21), tm, true);
		    l = 3 + 20;
		    if(valve_type == vt_6TU) {
			mPrm.vlAt(TSYS::strMess("TUstopEx_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 23), tm, true);
			mPrm.vlAt(TSYS::strMess("timeStopEx_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 25), tm, true);
			l += 4;
		    }

		}
		break;
	    case 2:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("tcOpen_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 3), tm, true);
		    mPrm.vlAt(TSYS::strMess("tcClose_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 5), tm, true);
		    mPrm.vlAt(TSYS::strMess("tcMode_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 7), tm, true);
		    mPrm.vlAt(TSYS::strMess("tcOpenErr_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 9), tm, true);
		    mPrm.vlAt(TSYS::strMess("tcCloseErr_%d", ft3ID.k)).at().setI(TSYS::getUnalign16(D + 11), tm, true);
		}
		l = 3 + 10;
		break;
	    case 3:
		mPrm.vlAt(TSYS::strMess("function_%d", ft3ID.k)).at().setI(D[3], tm, true);
		l = 4;
		break;

	    }
	}
    }

    return l;
}

uint8_t KA_GZD::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);
//    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "ID %d ft3ID g%d k%d n%d ", ID, ft3ID.g, ft3ID.k, ft3ID.n);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
    if(ft3ID.k == 0) {
	switch(ft3ID.n) {
	case 0:
	    //state
	    out[0] = 1;
	    l = 1;
	    break;
	case 1:
	    out[0] = config >> 8;
	    out[1] = config;
	    l = 2;
	    break;
	case 2:
	    for(uint8_t i = 0; i < count_n; i++) {
		out[i * 2] = i;
		out[i * 2 + 1] = data[i].State.vl;
	    }
	    l = count_n * 2;
	    break;
	}
    } else {
	if(ft3ID.k <= count_n) {
	    switch(ft3ID.n) {
	    case 0:
		out[0] = data[ft3ID.k - 1].State.s;
		out[1] = data[ft3ID.k - 1].State.vl;
		l = 2;
		break;
	    case 1:
		out[l++] = data[ft3ID.k - 1].TUOpen.s;
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TUOpen.b_vl[j];
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TimeOpen.b_vl[j];
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TUClose.b_vl[j];
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TimeClose.b_vl[j];
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TUStop.b_vl[j];
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TimeStop.b_vl[j];
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TURemote.b_vl[j];
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TimeRemote.b_vl[j];
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TUManual.b_vl[j];
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TimeManual.b_vl[j];
		if(valve_type == vt_6TU) {
		    for(uint8_t j = 0; j < 2; j++)
			out[l++] = data[ft3ID.k - 1].TUStopEx.b_vl[j];
		    for(uint8_t j = 0; j < 2; j++)
			out[l++] = data[ft3ID.k - 1].TimeStopEx.b_vl[j];
		}
		break;
	    case 2:
		out[l++] = data[ft3ID.k - 1].TCOpen.s;
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TCOpen.b_vl[j];
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TCClose.b_vl[j];
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TCMode.b_vl[j];
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TCOpenErr.b_vl[j];
		for(uint8_t j = 0; j < 2; j++)
		    out[l++] = data[ft3ID.k - 1].TCCloseErr.b_vl[j];
		break;
	    case 3:
		out[0] = data[ft3ID.k - 1].Function.s;
		out[1] = data[ft3ID.k - 1].Function.vl;
		l = 2;
		break;
	    }
	}
    }
    return l;
}

uint8_t KA_GZD::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
//    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "cmdSet k %d n %d", ft3ID.k, ft3ID.n);
    if(ft3ID.k <= count_n) {
	switch(ft3ID.n) {
	case 0:
	    l = data[ft3ID.k - 1].SetNewState(addr, prmID, req + 2);
	    break;
	case 1:
	    l = data[ft3ID.k - 1].SetNewTUParam(addr, prmID, req + 2);
	    break;
	case 2:
	    l = data[ft3ID.k - 1].SetNewTCParam(addr, prmID, req + 2);
	    break;
	case 3:
	    l = data[ft3ID.k - 1].SetNewFunction(addr, prmID, req + 2);
	}

    }
    return l;
}

uint16_t KA_GZD::setVal(TVal &val)
{
    uint16_t rc = 0;
    int off = 0;
    FT3ID ft3ID;
    ft3ID.k = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.n = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.g = ID;

    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
    if(count_n && (ft3ID.k <= count_n)) {
	switch(ft3ID.n) {
	case 0:
	    Msg.L += SerializeB(Msg.D + Msg.L, val.getI(0, true));
	    break;
	case 1:
	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("TUopen_%d", ft3ID.k)).at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("timeOpen_%d", ft3ID.k)).at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("TUclose_%d", ft3ID.k)).at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("timeClose_%d", ft3ID.k)).at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("TUstop_%d", ft3ID.k)).at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("timeStop_%d", ft3ID.k)).at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("TUremote_%d", ft3ID.k)).at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("timeRemote_%d", ft3ID.k)).at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("TUmanual_%d", ft3ID.k)).at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("timeManual_%d", ft3ID.k)).at().getI(0, true));
	    if(valve_type == vt_6TU) {
		Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("TUstopEx_%d", ft3ID.k)).at().getI(0, true));
		Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("timeStopEx_%d", ft3ID.k)).at().getI(0, true));
	    }
	    rc = 1;
	    break;
	case 2:

	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("tcOpen_%d", ft3ID.k)).at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("tcClose_%d", ft3ID.k)).at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("tcMode_%d", ft3ID.k)).at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("tcOpenErr_%d", ft3ID.k)).at().getI(0, true));
	    Msg.L += SerializeUi16(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("tcCloseErr_%d", ft3ID.k)).at().getI(0, true));
	    rc = 1;
	    break;
	case 3:
	    Msg.L += SerializeB(Msg.D + Msg.L, val.getI(0, true));
	    break;
	}
    }
    if(Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }
    return rc;
}
