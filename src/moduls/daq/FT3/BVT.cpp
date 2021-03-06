//OpenSCADA module DAQ.FT3 file: BVT.cpp
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
#include "BVT.h"

using namespace FT3;

void KA_BVT::SKATTchannel::UpdateTTParam(uint16_t ID, uint8_t cl)
{
    ui8fl tmpfl[11];
    uint8_t tmp;
    tmp = Period.Get();
    tmpfl[0].f = Sens.Get();
    tmpfl[1].f = MinS.Get();
    tmpfl[2].f = MaxS.Get();
    tmpfl[3].f = MinPV.Get();
    tmpfl[4].f = MaxPV.Get();
    tmpfl[5].f = MinA.Get();
    tmpfl[6].f = MaxA.Get();
    tmpfl[7].f = MinW.Get();
    tmpfl[8].f = MaxW.Get();
    tmpfl[9].f = Factor.Get();
    tmpfl[10].f = Adjust.Get();
    if(tmp != Period.vl || tmpfl[0].f != Sens.vl || tmpfl[1].f != MinS.vl || tmpfl[2].f != MaxS.vl || tmpfl[3].f != MinPV.vl || tmpfl[4].f != MaxPV.vl
	    || tmpfl[5].f != MinA.vl || tmpfl[6].f != MaxA.vl || tmpfl[7].f != MinW.vl || tmpfl[8].f != MaxW.vl || tmpfl[9].f != Factor.vl
	    || tmpfl[10].f != Adjust.vl) {
	Period.s = 0;
	Period.Update(tmp);
	Sens.Update(tmpfl[0].f, 0);
	MinS.Update(tmpfl[1].f, 0);
	MaxS.Update(tmpfl[2].f, 0);
	MinPV.Update(tmpfl[3].f, 0);
	MaxPV.Update(tmpfl[4].f, 0);
	MinA.Update(tmpfl[5].f, 0);
	MaxA.Update(tmpfl[6].f, 0);
	MinW.Update(tmpfl[7].f, 0);
	MaxW.Update(tmpfl[8].f, 0);
	Factor.Update(tmpfl[9].f, 0);
	Adjust.Update(tmpfl[10].f, 0);
	uint8_t E[46] = { 0, tmp, tmpfl[0].b[0], tmpfl[0].b[1], tmpfl[0].b[2], tmpfl[0].b[3], tmpfl[1].b[0], tmpfl[1].b[1], tmpfl[1].b[2], tmpfl[1].b[3],
		tmpfl[2].b[0], tmpfl[2].b[1], tmpfl[2].b[2], tmpfl[2].b[3], tmpfl[3].b[0], tmpfl[3].b[1], tmpfl[3].b[2], tmpfl[3].b[3], tmpfl[4].b[0],
		tmpfl[4].b[1], tmpfl[4].b[2], tmpfl[4].b[3], tmpfl[5].b[0], tmpfl[5].b[1], tmpfl[5].b[2], tmpfl[5].b[3], tmpfl[6].b[0], tmpfl[6].b[1],
		tmpfl[6].b[2], tmpfl[6].b[3], tmpfl[7].b[0], tmpfl[7].b[1], tmpfl[7].b[2], tmpfl[7].b[3], tmpfl[8].b[0], tmpfl[8].b[1], tmpfl[8].b[2],
		tmpfl[8].b[3], tmpfl[9].b[0], tmpfl[9].b[1], tmpfl[9].b[2], tmpfl[9].b[3], tmpfl[10].b[0], tmpfl[10].b[1], tmpfl[10].b[2], tmpfl[10].b[3] };
	da->PushInBE(cl, sizeof(E), ID, E);
    }
}

uint8_t KA_BVT::SKATTchannel::SetNewTTParam(uint8_t addr, uint16_t prmID, uint8_t *val)
{
    if((State.vl == 4) && (TSYS::getUnalignFloat(val + 9) > TSYS::getUnalignFloat(val + 5))
	    && (TSYS::getUnalignFloat(val + 17) > TSYS::getUnalignFloat(val + 13)) && (TSYS::getUnalignFloat(val + 25) > TSYS::getUnalignFloat(val + 21))
	    && (TSYS::getUnalignFloat(val + 33) > TSYS::getUnalignFloat(val + 29)) && (TSYS::getUnalignFloat(val + 37) >= 0)
	    && (0.02 > TSYS::getUnalignFloat(val + 37))) {
	if(Period.lnk.Connected() && Sens.lnk.Connected() && MinS.lnk.Connected() && MaxS.lnk.Connected() && MinPV.lnk.Connected() && MaxPV.lnk.Connected()
		&& MinA.lnk.Connected() && MaxA.lnk.Connected() && MinW.lnk.Connected() && MaxW.lnk.Connected() && Factor.lnk.Connected()
		&& Adjust.lnk.Connected()) {
	    Period.s = addr;
	    Period.Set(val[0]);
	    Sens.Set(TSYS::getUnalignFloat(val + 1));
	    MinS.Set(TSYS::getUnalignFloat(val + 5));
	    MaxS.Set(TSYS::getUnalignFloat(val + 9));
	    MinPV.Set(TSYS::getUnalignFloat(val + 13));
	    MaxPV.Set(TSYS::getUnalignFloat(val + 17));
	    MinA.Set(TSYS::getUnalignFloat(val + 21));
	    MaxA.Set(TSYS::getUnalignFloat(val + 25));
	    MinW.Set(TSYS::getUnalignFloat(val + 29));
	    MaxW.Set(TSYS::getUnalignFloat(val + 33));
	    Factor.Set(TSYS::getUnalignFloat(val + 37));
	    Adjust.Set(TSYS::getUnalignFloat(val + 41));
	    uint8_t E[46];
	    E[0] = addr;
	    memcpy(E + 1, val, 45);
	    da->PushInBE(1, sizeof(E), prmID, E);
	    return 2 + 45;
	} else {
	    return 0;
	}
    } else {
	return 0;
    }
}

KA_BVT::KA_BVT(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params) :
	DA(prm), ID(id), count_n(n), with_params(has_params), config(5 | (n << 4) | (4 << 10))
{
    mTypeFT3 = KA;
    chan_err.clear();
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    for(int i = 0; i < count_n; i++) {
	AddChannel(i);
    }
    loadIO(true);
}

KA_BVT::~KA_BVT()
{

}

void KA_BVT::AddChannel(uint8_t iid)
{
    chan_err.insert(chan_err.end(), SDataRec());
    data.push_back(SKATTchannel(iid, this));
    AddAttr(data.back().State.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
    AddAttr(data.back().Value.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:0", iid + 1));
    if(with_params) {
	AddAttr(data.back().Period.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Sens.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MinS.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MaxS.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MinPV.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MaxPV.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MinW.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MaxW.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MinA.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().MaxA.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Factor.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Adjust.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
    }
}

string KA_BVT::getStatus(void)
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

uint16_t KA_BVT::GetState()
{
    tagMsg Msg;
    uint16_t rc = BlckStateUnknown;
    Msg.L = 5;
    Msg.C = AddrReq;
    *((uint16_t *) Msg.D) = PackID(ID, 0, 0); //state
    if(mPrm.owner().DoCmd(&Msg) == GOOD3) {
	switch(mPrm.vlAt("state").at().getI(0, true)) {
	case KA_BVT_Error:
	    rc = BlckStateError;
	    break;
	case KA_BVT_Normal:
	    rc = BlckStateNormal;
	    break;
	}
    }
    return rc;
}

uint16_t KA_BVT::PreInit(void)
{
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    for(int i = 0; i < count_n; i++) {
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 1));
	Msg.L += SerializeB(Msg.D + Msg.L, TT_OFF);
    }
    Msg.L += 3;
    return mPrm.owner().DoCmd(&Msg);
}

uint16_t KA_BVT::SetParams(void)
{
    uint16_t rc;
    tagMsg Msg;
    loadParam();
    for(int i = 0; i < count_n; i++) {
	if(data[i].State.lnk.vlattr.at().getI(0, true) != TT_OFF) {
	    Msg.L = 0;
	    Msg.C = SetData;
	    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 2));
	    Msg.L += SerializeB(Msg.D + Msg.L, data[i].Period.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeF(Msg.D + Msg.L, data[i].Sens.lnk.vlattr.at().getR(0, true));
	    Msg.L += SerializeF(Msg.D + Msg.L, data[i].MinS.lnk.vlattr.at().getR(0, true));
	    Msg.L += SerializeF(Msg.D + Msg.L, data[i].MaxS.lnk.vlattr.at().getR(0, true));
	    Msg.L += SerializeF(Msg.D + Msg.L, data[i].MinPV.lnk.vlattr.at().getR(0, true));
	    Msg.L += SerializeF(Msg.D + Msg.L, data[i].MaxPV.lnk.vlattr.at().getR(0, true));
	    Msg.L += SerializeF(Msg.D + Msg.L, data[i].MinW.lnk.vlattr.at().getR(0, true));
	    Msg.L += SerializeF(Msg.D + Msg.L, data[i].MaxW.lnk.vlattr.at().getR(0, true));
	    Msg.L += SerializeF(Msg.D + Msg.L, data[i].MinA.lnk.vlattr.at().getR(0, true));
	    Msg.L += SerializeF(Msg.D + Msg.L, data[i].MaxA.lnk.vlattr.at().getR(0, true));
	    Msg.L += SerializeF(Msg.D + Msg.L, data[i].Factor.lnk.vlattr.at().getR(0, true));
	    Msg.L += SerializeF(Msg.D + Msg.L, data[i].Adjust.lnk.vlattr.at().getR(0, true));
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
    }
    return rc;

}

uint16_t KA_BVT::RefreshParams(void)
{
    uint16_t rc;
    tagMsg Msg;
    for(int i = 1; i <= count_n; i++) {
	Msg.L = 0;
	Msg.C = AddrReq;
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

uint16_t KA_BVT::PostInit(void)
{
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    for(int i = 0; i < count_n; i++) {
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 1));
	Msg.L += SerializeB(Msg.D + Msg.L, data[i].State.lnk.vlattr.at().getI(0, true));
    }
    Msg.L += 3;
    return mPrm.owner().DoCmd(&Msg);
}

uint16_t KA_BVT::RefreshData(void)
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

void KA_BVT::loadIO(bool force)
{
    if(mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }	//Load/reload IO context only allow for stopped controllers for prevent throws
    for(int i = 0; i < count_n; i++) {
	loadLnk(data[i].State.lnk);
	loadLnk(data[i].Value.lnk);
	loadLnk(data[i].Period.lnk);
	loadLnk(data[i].Sens.lnk);
	loadLnk(data[i].MinS.lnk);
	loadLnk(data[i].MaxS.lnk);
	loadLnk(data[i].MinPV.lnk);
	loadLnk(data[i].MaxPV.lnk);
	loadLnk(data[i].MinW.lnk);
	loadLnk(data[i].MaxW.lnk);
	loadLnk(data[i].MinA.lnk);
	loadLnk(data[i].MaxA.lnk);
	loadLnk(data[i].Factor.lnk);
	loadLnk(data[i].Adjust.lnk);
    }
}

void KA_BVT::saveIO(void)
{
    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "save io");
    for(int i = 0; i < count_n; i++) {
	saveLnk(data[i].State.lnk);
	saveLnk(data[i].Value.lnk);
	saveLnk(data[i].Period.lnk);
	saveLnk(data[i].Sens.lnk);
	saveLnk(data[i].MinS.lnk);
	saveLnk(data[i].MaxS.lnk);
	saveLnk(data[i].MinPV.lnk);
	saveLnk(data[i].MaxPV.lnk);
	saveLnk(data[i].MinW.lnk);
	saveLnk(data[i].MaxW.lnk);
	saveLnk(data[i].MinA.lnk);
	saveLnk(data[i].MaxA.lnk);
	saveLnk(data[i].Factor.lnk);
	saveLnk(data[i].Adjust.lnk);
    }
}

void KA_BVT::saveParam(void)
{
    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "save param");
    for(int i = 0; i < count_n; i++) {
	saveVal(data[i].State.lnk);
	saveVal(data[i].Period.lnk);
	saveVal(data[i].Sens.lnk);
	saveVal(data[i].MinS.lnk);
	saveVal(data[i].MaxS.lnk);
	saveVal(data[i].MinPV.lnk);
	saveVal(data[i].MaxPV.lnk);
	saveVal(data[i].MinW.lnk);
	saveVal(data[i].MaxW.lnk);
	saveVal(data[i].MinA.lnk);
	saveVal(data[i].MaxA.lnk);
	saveVal(data[i].Factor.lnk);
	saveVal(data[i].Adjust.lnk);
    }
}

void KA_BVT::loadParam(void)
{
    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "load param");
    for(int i = 0; i < count_n; i++) {
	loadVal(data[i].State.lnk);
	loadVal(data[i].Period.lnk);
	loadVal(data[i].Sens.lnk);
	loadVal(data[i].MinS.lnk);
	loadVal(data[i].MaxS.lnk);
	loadVal(data[i].MinPV.lnk);
	loadVal(data[i].MaxPV.lnk);
	loadVal(data[i].MinW.lnk);
	loadVal(data[i].MaxW.lnk);
	loadVal(data[i].MinA.lnk);
	loadVal(data[i].MaxA.lnk);
	loadVal(data[i].Factor.lnk);
	loadVal(data[i].Adjust.lnk);
    }
}

void KA_BVT::tmHandler(void)
{
    for(int i = 0; i < count_n; i++) {
	if(with_params) {
	    data[i].UpdateTTParam(PackID(ID, (i + 1), 2), 1);
	}
	UpdateParamFlState(data[i].Value, data[i].State, data[i].Sens, PackID(ID, (i + 1), 0), 2);
    }
    NeedInit = false;
}

uint16_t KA_BVT::HandleEvent(int64_t tm, uint8_t * D)
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
	    l = 2 + count_n * 5;
	    for(int j = 0; j < count_n; j++) {
		mPrm.vlAt(TSYS::strMess("state_%d", j)).at().setI(D[j * 5 + 2], tm, true);
		mPrm.vlAt(TSYS::strMess("value_%d", j)).at().setR(TSYS::getUnalignFloat(D + j * 5 + 3), tm, true);
	    }
	    break;
	}
	break;
    default:
	if(ft3ID.k && (ft3ID.k <= count_n)) {
	    switch(ft3ID.n) {
	    case 0:
		mPrm.vlAt(TSYS::strMess("state_%d", ft3ID.k)).at().setI(D[2], tm, true);
		mPrm.vlAt(TSYS::strMess("value_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
		l = 7;
		break;
	    case 1:
		mPrm.vlAt(TSYS::strMess("state_%d", ft3ID.k)).at().setI(D[3], tm, true);
		l = 4;
		break;
	    case 2:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("period_%d", ft3ID.k)).at().setI(D[3], tm, true);
		    mPrm.vlAt(TSYS::strMess("sens_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 4), tm, true);
		    mPrm.vlAt(TSYS::strMess("minS_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 8), tm, true);
		    mPrm.vlAt(TSYS::strMess("maxS_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 12), tm, true);
		    mPrm.vlAt(TSYS::strMess("minPV_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 16), tm, true);
		    mPrm.vlAt(TSYS::strMess("maxPV_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 20), tm, true);
		    mPrm.vlAt(TSYS::strMess("minA_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 24), tm, true);
		    mPrm.vlAt(TSYS::strMess("maxA_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 28), tm, true);
		    mPrm.vlAt(TSYS::strMess("minW_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 32), tm, true);
		    mPrm.vlAt(TSYS::strMess("maxW_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 36), tm, true);
		    mPrm.vlAt(TSYS::strMess("factor_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 40), tm, true);
		    mPrm.vlAt(TSYS::strMess("adjust_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 44), tm, true);
		}
		l = 48;
		break;

	    }
	}
    }

    return l;
}

uint8_t KA_BVT::cmdGet(uint16_t prmID, uint8_t * out)
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
		out[i * 5] = data[i].State.vl;
		for(uint8_t j = 0; j < 4; j++)
		    out[i * 5 + j + 1] = data[i].Value.b_vl[j];
		l += 5;
	    }
	    break;
	}
    } else {
	if(ft3ID.k <= count_n) {
	    switch(ft3ID.n) {
	    case 0:
		out[0] = data[ft3ID.k - 1].State.vl;
		for(uint8_t j = 0; j < 4; j++)
		    out[1 + j] = data[ft3ID.k - 1].Value.b_vl[j];
		l = 5;
		break;
	    case 1:
		out[0] = data[ft3ID.k - 1].State.s;
		out[1] = data[ft3ID.k - 1].State.vl;
		l = 2;
		break;
	    case 2:
		out[0] = data[ft3ID.k - 1].Period.s;
		out[1] = data[ft3ID.k - 1].Period.vl;
		l = 2;
		for(uint8_t j = 0; j < 4; j++)
		    out[l++] = data[ft3ID.k - 1].Sens.b_vl[j];
		for(uint8_t j = 0; j < 4; j++)
		    out[l++] = data[ft3ID.k - 1].MinS.b_vl[j];
		for(uint8_t j = 0; j < 4; j++)
		    out[l++] = data[ft3ID.k - 1].MaxS.b_vl[j];
		for(uint8_t j = 0; j < 4; j++)
		    out[l++] = data[ft3ID.k - 1].MinPV.b_vl[j];
		for(uint8_t j = 0; j < 4; j++)
		    out[l++] = data[ft3ID.k - 1].MaxPV.b_vl[j];
		for(uint8_t j = 0; j < 4; j++)
		    out[l++] = data[ft3ID.k - 1].MinA.b_vl[j];
		for(uint8_t j = 0; j < 4; j++)
		    out[l++] = data[ft3ID.k - 1].MaxA.b_vl[j];
		for(uint8_t j = 0; j < 4; j++)
		    out[l++] = data[ft3ID.k - 1].MinW.b_vl[j];
		for(uint8_t j = 0; j < 4; j++)
		    out[l++] = data[ft3ID.k - 1].MaxW.b_vl[j];
		for(uint8_t j = 0; j < 4; j++)
		    out[l++] = data[ft3ID.k - 1].Factor.b_vl[j];
		for(uint8_t j = 0; j < 4; j++)
		    out[l++] = data[ft3ID.k - 1].Adjust.b_vl[j];
	    }

	}
    }
    return l;
}

uint8_t KA_BVT::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
//    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "cmdSet k %d n %d", ft3ID.k, ft3ID.n);
    if(ft3ID.k <= count_n) {
	switch(ft3ID.n) {
	case 0:
	    if((data[ft3ID.k - 1].State.vl == 4)) {
		l = SetNewflVal(data[ft3ID.k - 1].Value, data[ft3ID.k - 1].State.vl, prmID, TSYS::getUnalignFloat(req + 2));
	    }
	    break;
	case 1:
	    l = SetNew8Val(data[ft3ID.k - 1].State, addr, prmID, req[2]);
	    break;
	case 2:
	    l = data[ft3ID.k - 1].SetNewTTParam(addr, prmID, req + 2);
	    break;
	}
    }
    return l;
}

uint16_t KA_BVT::setVal(TVal &val)
{
    uint16_t rc = 0;
    int off = 0;
    FT3ID ft3ID;
    ft3ID.k = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.n = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.g = ID;

    off = 0;
    FT3ID stateft3ID;
    stateft3ID.k = s2i(TSYS::strParse(mPrm.vlAt(TSYS::strMess("state_%d", ft3ID.k)).at().fld().reserve(), 0, ":", &off));
    stateft3ID.n = s2i(TSYS::strParse(mPrm.vlAt(TSYS::strMess("state_%d", ft3ID.k)).at().fld().reserve(), 0, ":", &off));
    stateft3ID.g = ID;

    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
    switch(ft3ID.n) {
    case 0:
	Msg.L = 0;
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(stateft3ID));
	Msg.L += SerializeB(Msg.D + Msg.L, 4);
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
	Msg.L += SerializeF(Msg.D + Msg.L, val.getR(0, true));
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(stateft3ID));
	Msg.L += SerializeB(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("state_%d", ft3ID.k)).at().getI(0, true));
	break;
    case 1:
	Msg.L += SerializeB(Msg.D + Msg.L, val.getI(0, true));
	break;
    case 2:
	Msg.L += SerializeB(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("period_%d", ft3ID.k)).at().getI(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("sens_%d", ft3ID.k)).at().getR(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("minS_%d", ft3ID.k)).at().getR(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("maxS_%d", ft3ID.k)).at().getR(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("minPV_%d", ft3ID.k)).at().getR(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("maxPV_%d", ft3ID.k)).at().getR(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("minA_%d", ft3ID.k)).at().getR(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("maxA_%d", ft3ID.k)).at().getR(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("minW_%d", ft3ID.k)).at().getR(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("maxW_%d", ft3ID.k)).at().getR(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("factor_%d", ft3ID.k)).at().getR(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("adjust_%d", ft3ID.k)).at().getR(0, true));
	rc = 1;
	break;
    }
    if(Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }
    return rc;
}

B_BVT::B_BVT(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params, bool has_k, bool has_rate) :
	DA(prm), ID(id), count_n(n), with_params(has_params), with_k(has_k), with_rate(has_rate)
{
    mTypeFT3 = GRS;
    blkID = 0x10;
    chan_err.clear();
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    for(int i = 0; i < count_n; i++) {
	AddChannel(i);
    }
    loadIO(true);
}

B_BVT::~B_BVT()
{

}

void B_BVT::AddChannel(uint8_t iid)
{
    chan_err.insert(chan_err.end(), SDataRec());
    data.push_back(STTchannel(iid, this));
    AddAttr(data.back().State.lnk, TFld::Integer, TFld::NoWrite, TSYS::strMess("%d:0", iid + 1));
    AddAttr(data.back().Value.lnk, TFld::Real, TFld::NoWrite, TSYS::strMess("%d:1", iid + 1));
    if(with_params) {
	AddAttr(data.back().Period.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:2", iid + 1));
	AddAttr(data.back().Sens.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:3", iid + 1));
	AddAttr(data.back().MinS.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:4", iid + 1));
	AddAttr(data.back().MaxS.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:4", iid + 1));
	AddAttr(data.back().MinPV.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:5", iid + 1));
	AddAttr(data.back().MaxPV.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:5", iid + 1));
	AddAttr(data.back().MinW.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:6", iid + 1));
	AddAttr(data.back().MaxW.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:6", iid + 1));
	AddAttr(data.back().MinA.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:7", iid + 1));
	AddAttr(data.back().MaxA.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:7", iid + 1));
	AddAttr(data.back().Factor.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:8", iid + 1));
	AddAttr(data.back().Dimension.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:9", iid + 1));
	if(with_k) {
	    AddAttr(data.back().CorFactor.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:10", iid + 1));
	    if(with_rate) {
		AddAttr(data.back().Rate.lnk, TFld::Real, TFld::NoWrite, TSYS::strMess("%d:11", iid + 1));
		AddAttr(data.back().Calcs.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:12", iid + 1));
		AddAttr(data.back().RateSens.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:13", iid + 1));
		AddAttr(data.back().RateLimit.lnk, TFld::Real, TVal::DirWrite, TSYS::strMess("%d:14", iid + 1));

	    }
	}
    }
}

string B_BVT::getStatus(void)
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

void B_BVT::loadIO(bool force)
{
    if(mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }	//Load/reload IO context only allow for stopped controllers for prevent throws

    for(int i = 0; i < count_n; i++) {
	loadLnk(data[i].State.lnk);
	loadLnk(data[i].Value.lnk);
	loadLnk(data[i].Period.lnk);
	loadLnk(data[i].Sens.lnk);
	loadLnk(data[i].MinS.lnk);
	loadLnk(data[i].MaxS.lnk);
	loadLnk(data[i].MinPV.lnk);
	loadLnk(data[i].MaxPV.lnk);
	loadLnk(data[i].MinW.lnk);
	loadLnk(data[i].MaxW.lnk);
	loadLnk(data[i].MinA.lnk);
	loadLnk(data[i].MaxA.lnk);
	loadLnk(data[i].Factor.lnk);
	loadLnk(data[i].Dimension.lnk);
	loadLnk(data[i].CorFactor.lnk);
	loadLnk(data[i].Rate.lnk);
	loadLnk(data[i].Calcs.lnk);
	loadLnk(data[i].RateSens.lnk);
	loadLnk(data[i].RateLimit.lnk);
    }
}

void B_BVT::saveIO()
{
    for(int i = 0; i < count_n; i++) {
	saveLnk(data[i].State.lnk);
	saveLnk(data[i].Value.lnk);
	saveLnk(data[i].Period.lnk);
	saveLnk(data[i].Sens.lnk);
	saveLnk(data[i].MinS.lnk);
	saveLnk(data[i].MaxS.lnk);
	saveLnk(data[i].MinPV.lnk);
	saveLnk(data[i].MaxPV.lnk);
	saveLnk(data[i].MinW.lnk);
	saveLnk(data[i].MaxW.lnk);
	saveLnk(data[i].MinA.lnk);
	saveLnk(data[i].MaxA.lnk);
	saveLnk(data[i].Factor.lnk);
	saveLnk(data[i].Dimension.lnk);
	saveLnk(data[i].CorFactor.lnk);
	saveLnk(data[i].Rate.lnk);
	saveLnk(data[i].Calcs.lnk);
	saveLnk(data[i].RateSens.lnk);
	saveLnk(data[i].RateLimit.lnk);
    }
}

void B_BVT::tmHandler(void)
{
    for(int i = 0; i < count_n; i++) {
	if(with_params) {
	    UpdateParam8(data[i].Period, PackID(ID, (i + 1), 2), 1);
	    UpdateParamFl(data[i].Sens, PackID(ID, (i + 1), 3), 1);
	    UpdateParam2Fl(data[i].MinS, data[i].MaxS, PackID(ID, (i + 1), 4), 1);
	    UpdateParam2Fl(data[i].MinPV, data[i].MaxPV, PackID(ID, (i + 1), 5), 1);
	    UpdateParam2Fl(data[i].MinW, data[i].MaxW, PackID(ID, (i + 1), 6), 1);
	    UpdateParam2Fl(data[i].MinA, data[i].MaxA, PackID(ID, (i + 1), 7), 1);
	    UpdateParamFl(data[i].Factor, PackID(ID, (i + 1), 8), 1);
	    UpdateParam8(data[i].Dimension, PackID(ID, (i + 1), 9), 1);
	    if(with_k) {
		UpdateParamFl(data[i].CorFactor, PackID(ID, (i + 1), 10), 1);
		if(with_rate) {
		    UpdateParamFlState(data[i].Rate, data[i].State, data[i].RateSens, PackID(ID, (i + 1), 11), 2);
		    UpdateParam8(data[i].Calcs, PackID(ID, (i + 1), 12), 1);
		    UpdateParamFl(data[i].RateSens, PackID(ID, (i + 1), 13), 1);
		    UpdateParamFl(data[i].RateLimit, PackID(ID, (i + 1), 14), 1);
		}
	    }
	}
	UpdateParamFlState(data[i].Value, data[i].State, data[i].Sens, PackID(ID, (i + 1), 1), 2);
    }
    NeedInit = false;
}

uint16_t B_BVT::Task(uint16_t uc)
{
    tagMsg Msg;
    uint16_t rc = 0;
    switch(uc) {
    case TaskRefresh:
	Msg.L = 5;
	Msg.C = AddrReq;
	*((uint16_t *) Msg.D) = PackID(ID, 0, 0); //??????????????????
	if(mPrm.owner().DoCmd(&Msg)) {
	    if(Msg.C == GOOD3) {
		NeedInit = false;
		if(with_params) {
		    for(int i = 1; i <= count_n; i++) {
			if(chan_err[i].state == 1) continue;
			Msg.L = 21;
			Msg.C = AddrReq;
			*((uint16_t *) Msg.D) = PackID(ID, i, 1); //???????????????? ??T
			*((uint16_t *) (Msg.D + 2)) = PackID(ID, i, 2); //???????????? ??????????????????
			*((uint16_t *) (Msg.D + 4)) = PackID(ID, i, 3); //????????????????????????????????
			*((uint16_t *) (Msg.D + 6)) = PackID(ID, i, 4); //min max ??????????????
			*((uint16_t *) (Msg.D + 8)) = PackID(ID, i, 5); //min max ????
			*((uint16_t *) (Msg.D + 10)) = PackID(ID, i, 6); //min max ??????????????????????????????????
			*((uint16_t *) (Msg.D + 12)) = PackID(ID, i, 7); //min max ??????????????????
			*((uint16_t *) (Msg.D + 14)) = PackID(ID, i, 8); //?????????????????????? ????????????????????
			*((uint16_t *) (Msg.D + 16)) = PackID(ID, i, 9); //??????????????????????
			if(with_k) {
			    *((uint16_t *) (Msg.D + 18)) = PackID(ID, i, 10); //???????????????????????????? ??????????????????????
			    Msg.L += 2;
			    if(with_rate) {
				*((uint16_t *) (Msg.D + 20)) = PackID(ID, i, 11); //?????????????? ????????????????
				*((uint16_t *) (Msg.D + 22)) = PackID(ID, i, 12); //???????????????????? ????????????????????
				*((uint16_t *) (Msg.D + 24)) = PackID(ID, i, 13); //????????????????????????????????
				*((uint16_t *) (Msg.D + 26)) = PackID(ID, i, 14); //???????????????????? ????????????????
				Msg.L += 8;
			    }
			}

			if(mPrm.owner().DoCmd(&Msg)) {
			    if(Msg.C == GOOD3) {
				chan_err[i].state = 1;
				rc = 1;
			    } else {
				rc = 0;
				chan_err[i].state = 2;
				NeedInit = true;
			    }
			} else {
			    rc = 0;
			    chan_err[i].state = 3;
			    NeedInit = true;
			}

		    }
		} else {
		    rc = 1;
		}
	    } else {
		rc = 0;
		NeedInit = true;
	    }
	}
	break;
    }
    return rc;
}

uint16_t B_BVT::HandleEvent(int64_t tm, uint8_t * D)
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
	    mPrm.vlAt("state").at().setI(D[2], tm, true);
	    l = 3 + count_n * 5;
	    for(int j = 1; j <= count_n; j++) {
		mPrm.vlAt(TSYS::strMess("state_%d", j)).at().setI(D[(j - 1) * 5 + 3], tm, true);
		mPrm.vlAt(TSYS::strMess("value_%d", j)).at().setR(TSYS::getUnalignFloat(D + (j - 1) * 5 + 4), tm, true);
	    }
	    break;
	case 2:
	    mPrm.vlAt("state").at().setI(D[2], 0, true);
	    l = 3 + count_n * 5;
	    for(int j = 1; j <= count_n; j++) {
		mPrm.vlAt(TSYS::strMess("state_%d", j)).at().setI(D[(j - 1) * 5 + 3], 0, true);
		mPrm.vlAt(TSYS::strMess("ratelimit_%d", j)).at().setR(TSYS::getUnalignFloat(D + (j - 1) * 5 + 4), tm, true);
	    }
	    break;

	}
	break;
    default:
	if(ft3ID.k && (ft3ID.k <= count_n)) {
	    switch(ft3ID.n) {
	    case 0:
		mPrm.vlAt(TSYS::strMess("state_%d", ft3ID.k)).at().setI(D[2], tm, true);
		l = 3;
		break;
	    case 1:
		mPrm.vlAt(TSYS::strMess("state_%d", ft3ID.k)).at().setI(D[2], tm, true);
		mPrm.vlAt(TSYS::strMess("value_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);

		l = 7;
		break;
	    case 2:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("period_%d", ft3ID.k)).at().setI(D[3], tm, true);
		}
		l = 4;
		break;
	    case 3:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("sens_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
		}
		l = 7;
		break;
	    case 4:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("minS_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
		    mPrm.vlAt(TSYS::strMess("maxS_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 7), tm, true);
		}
		l = 11;
		break;
	    case 5:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("minPV_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
		    mPrm.vlAt(TSYS::strMess("maxPV_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 7), tm, true);
		}
		l = 11;
		break;
	    case 6:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("minW_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
		    mPrm.vlAt(TSYS::strMess("maxW_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 7), tm, true);
		}
		l = 11;
		break;
	    case 7:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("minA_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
		    mPrm.vlAt(TSYS::strMess("maxA_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 7), tm, true);
		}
		l = 11;
		break;
	    case 8:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("factor_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
		}
		l = 7;
		break;
	    case 9:
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("dimens_%d", ft3ID.k)).at().setI(D[3], tm, true);
		}
		l = 4;
		break;
	    case 10:
		if(with_params && with_k) {
		    mPrm.vlAt(TSYS::strMess("corfactor_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
		}
		l = 7;
		break;
	    case 11:
		if(with_params && with_rate) {
		    mPrm.vlAt(TSYS::strMess("rate_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
		}
		l = 7;
		break;
	    case 12:
		if(with_params && with_rate) {
		    mPrm.vlAt(TSYS::strMess("calcs_%d", ft3ID.k)).at().setI(D[3], tm, true);
		}
		l = 4;
		break;
	    case 13:
		if(with_params && with_rate) {
		    mPrm.vlAt(TSYS::strMess("ratesens_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
		}
		l = 7;
		break;
	    case 14:
		if(with_params && with_rate) {
		    mPrm.vlAt(TSYS::strMess("ratelimit_%d", ft3ID.k)).at().setR(TSYS::getUnalignFloat(D + 3), tm, true);
		}
		l = 7;
		break;
	    }
	}
	break;
    }
    return l;
}

uint8_t B_BVT::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
    if(ft3ID.k == 0) {
	switch(ft3ID.n) {
	case 0:
	    //state
	    out[0] = 0 | blkID;
	    l = 1;
	    break;
	case 1:

	    out[0] = 0 | blkID;
	    l = 1;
	    //value
	    for(uint8_t i = 0; i < count_n; i++) {
		out[i * 5 + 1] = data[i].State.vl;
		for(uint8_t j = 0; j < 4; j++)
		    out[i * 5 + 2 + j] = data[i].Value.b_vl[j];
		l += 5;
	    }
	    break;
	case 2:
	    out[0] = 0 | blkID;
	    l = 1;
	    //rate
	    for(uint8_t i = 0; i < count_n; i++) {
		out[i * 5 + 1] = data[i].State.vl;
		for(uint8_t j = 0; j < 4; j++)
		    out[i * 5 + 2 + j] = data[i].Rate.b_vl[j];
		l += 5;
	    }
	    break;
	}
    } else {
	if(ft3ID.k <= count_n) {
	    switch(ft3ID.n) {
	    case 0:
		out[0] = data[ft3ID.k - 1].State.vl;
		l = 1;
		break;
	    case 1:
		out[0] = data[ft3ID.k - 1].State.vl;
		for(uint8_t j = 0; j < 4; j++)
		    out[1 + j] = data[ft3ID.k - 1].Value.b_vl[j];
		l = 5;
		break;
	    case 2:
		out[0] = data[ft3ID.k - 1].Period.s;
		out[1] = data[ft3ID.k - 1].Period.vl;
		l = 2;
		break;
	    case 3:
		out[0] = data[ft3ID.k - 1].Sens.s;
		for(uint8_t j = 0; j < 4; j++)
		    out[1 + j] = data[ft3ID.k - 1].Sens.b_vl[j];
		l = 5;
		break;
	    case 4:
		out[0] = data[ft3ID.k - 1].MinS.s;
		for(uint8_t j = 0; j < 4; j++) {
		    out[1 + j] = data[ft3ID.k - 1].MinS.b_vl[j];
		    out[5 + j] = data[ft3ID.k - 1].MaxS.b_vl[j];
		}
		l = 9;
		break;
	    case 5:
		out[0] = data[ft3ID.k - 1].MinPV.s;
		for(uint8_t j = 0; j < 4; j++) {
		    out[1 + j] = data[ft3ID.k - 1].MinPV.b_vl[j];
		    out[5 + j] = data[ft3ID.k - 1].MaxPV.b_vl[j];
		}
		l = 9;
		break;
	    case 6:
		out[0] = data[ft3ID.k - 1].MinW.s;
		for(uint8_t j = 0; j < 4; j++) {
		    out[1 + j] = data[ft3ID.k - 1].MinW.b_vl[j];
		    out[5 + j] = data[ft3ID.k - 1].MaxW.b_vl[j];
		}
		l = 9;
		break;
	    case 7:
		out[0] = data[ft3ID.k - 1].MinA.s;
		for(uint8_t j = 0; j < 4; j++) {
		    out[1 + j] = data[ft3ID.k - 1].MinA.b_vl[j];
		    out[5 + j] = data[ft3ID.k - 1].MaxA.b_vl[j];
		}
		l = 9;
		break;
	    case 8:
		out[0] = data[ft3ID.k - 1].Factor.s;
		for(uint8_t j = 0; j < 4; j++) {
		    out[1 + j] = data[ft3ID.k - 1].Factor.b_vl[j];
		}
		l = 5;
		break;
	    case 9:
		out[0] = data[ft3ID.k - 1].Dimension.s;
		out[1] = data[ft3ID.k - 1].Dimension.vl;
		l = 2;
		break;
	    case 10:
		out[0] = data[ft3ID.k - 1].CorFactor.s;
		for(uint8_t j = 0; j < 4; j++) {
		    out[1 + j] = data[ft3ID.k - 1].CorFactor.b_vl[j];
		}
		l = 5;
		break;
	    case 11:
		out[0] = data[ft3ID.k - 1].State.vl;
		for(uint8_t j = 0; j < 4; j++)
		    out[1 + j] = data[ft3ID.k - 1].Rate.b_vl[j];
		l = 5;
		break;
	    case 12:
		out[0] = data[ft3ID.k - 1].Calcs.s;
		out[1] = data[ft3ID.k - 1].Calcs.vl;
		l = 2;
		break;
	    case 13:
		out[0] = data[ft3ID.k - 1].RateSens.s;
		for(uint8_t j = 0; j < 4; j++)
		    out[1 + j] = data[ft3ID.k - 1].RateSens.b_vl[j];
		l = 5;
		break;
	    case 14:
		out[0] = data[ft3ID.k - 1].RateLimit.s;
		for(uint8_t j = 0; j < 4; j++)
		    out[1 + j] = data[ft3ID.k - 1].RateLimit.b_vl[j];
		l = 5;
		break;
	    }

	}
    }
    return l;
}

uint8_t B_BVT::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
//    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "cmdSet k %d n %d", ft3ID.k, ft3ID.n);
    if((ft3ID.k > 0) && (ft3ID.k <= count_n)) {
	switch(ft3ID.n) {
	case 2:
	    l = SetNew8Val(data[ft3ID.k - 1].Period, addr, prmID, req[2]);
	    break;
	case 3:
	    l = SetNewflVal(data[ft3ID.k - 1].Sens, addr, prmID, TSYS::getUnalignFloat(req + 2));
	    break;
	case 4:
	    l = SetNew2flVal(data[ft3ID.k - 1].MinS, data[ft3ID.k - 1].MaxS, addr, prmID, TSYS::getUnalignFloat(req + 2), TSYS::getUnalignFloat(req + 6));
	    break;
	case 5:
	    l = SetNew2flVal(data[ft3ID.k - 1].MinPV, data[ft3ID.k - 1].MaxPV, addr, prmID, TSYS::getUnalignFloat(req + 2), TSYS::getUnalignFloat(req + 6));
	    break;
	case 6:
	    l = SetNew2flVal(data[ft3ID.k - 1].MinW, data[ft3ID.k - 1].MaxW, addr, prmID, TSYS::getUnalignFloat(req + 2), TSYS::getUnalignFloat(req + 6));
	    break;
	case 7:
	    l = SetNew2flVal(data[ft3ID.k - 1].MinA, data[ft3ID.k - 1].MaxA, addr, prmID, TSYS::getUnalignFloat(req + 2), TSYS::getUnalignFloat(req + 6));
	    break;
	case 8:
	    l = SetNewflVal(data[ft3ID.k - 1].Factor, addr, prmID, TSYS::getUnalignFloat(req + 2));
	    break;
	case 9:
	    l = SetNew8Val(data[ft3ID.k - 1].Dimension, addr, prmID, req[2]);
	    break;
	case 10:
	    l = SetNewflVal(data[ft3ID.k - 1].CorFactor, addr, prmID, TSYS::getUnalignFloat(req + 2));
	    break;
	case 12:
	    l = SetNew8Val(data[ft3ID.k - 1].Calcs, addr, prmID, req[2]);
	    break;
	case 13:
	    l = SetNewflVal(data[ft3ID.k - 1].RateSens, addr, prmID, TSYS::getUnalignFloat(req + 2));
	    break;
	case 14:
	    l = SetNewflVal(data[ft3ID.k - 1].RateLimit, addr, prmID, TSYS::getUnalignFloat(req + 2));
	    break;
	}
    }
    return l;
}

uint16_t B_BVT::setVal(TVal &val)
{
    int off = 0;
    FT3ID ft3ID;
    ft3ID.k = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.n = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.g = ID;

    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
    switch(ft3ID.n) {
    case 2:
    case 9:
    case 12:
	Msg.L += SerializeB(Msg.D + Msg.L, val.getI(0, true));
	break;
    case 3:
    case 8:
    case 10:
    case 13:
    case 14:
	Msg.L += SerializeF(Msg.D + Msg.L, val.getR(0, true));
	break;
    case 4:
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("minS_%d", ft3ID.k)).at().getR(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("maxS_%d", ft3ID.k)).at().getR(0, true));
	break;
    case 5:
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("minPV_%d", ft3ID.k)).at().getR(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("maxPV_%d", ft3ID.k)).at().getR(0, true));
	break;
    case 6:
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("minW_%d", ft3ID.k)).at().getR(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("maxW_%d", ft3ID.k)).at().getR(0, true));
	break;
    case 7:
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("minA_%d", ft3ID.k)).at().getR(0, true));
	Msg.L += SerializeF(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("maxA_%d", ft3ID.k)).at().getR(0, true));
	break;
    }
    if(Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }
    return 0;
}

//---------------------------------------------------------------------------
