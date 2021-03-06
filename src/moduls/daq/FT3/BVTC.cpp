//OpenSCADA module DAQ.FT3 file: BVTC.cpp
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

#include "mod_FT3.h"
#include "BVTC.h"

using namespace FT3;
//******************************************************
//* B_BVTC                                             *
//******************************************************
KA_BVTC::KA_BVTC(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params) :
	DA(prm), count_n(n), ID(id), with_params(has_params), config(3 | (n << 4) | (3 << 10))
{
    mTypeFT3 = KA;
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    for(int i = 0; i < count_n; i++) {
	AddChannel(i);
    }
    loadIO(true);
}

KA_BVTC::~KA_BVTC()
{
    data.clear();
}

void KA_BVTC::AddChannel(uint8_t iid)
{
    data.push_back(SKATCchannel(iid, this));
    AddAttr(data.back().Value.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:0", iid + 1));
    if(with_params) {
	AddAttr(data.back().Period.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
	AddAttr(data.back().Count.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("%d:1", iid + 1));
    }
}

string KA_BVTC::getStatus(void)
{
    string rez;
    if(NeedInit) {
	rez = "20: ??????????";
    } else {
	rez = "0: ??????????";
    }
    return rez;
}

uint16_t KA_BVTC::GetState()
{
    tagMsg Msg;
    uint16_t rc = BlckStateUnknown;
    Msg.L = 5;
    Msg.C = AddrReq;
    *((uint16_t *) Msg.D) = PackID(ID, 0, 0); //state
    if(mPrm.owner().DoCmd(&Msg) == GOOD3) {
	switch(mPrm.vlAt("state").at().getI(0, true)) {
	case KA_BVTC_Error:
	    rc = BlckStateError;
	    break;
	case KA_BVTC_Normal:
	    rc = BlckStateNormal;
	    break;
	}
    }
    return rc;
}

uint16_t KA_BVTC::PreInit(void)
{
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    for(int i = 0; i < count_n; i++) {
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 0));
	Msg.L += SerializeB(Msg.D + Msg.L, TC_DISABLED);
    }
    Msg.L += 3;
    return mPrm.owner().DoCmd(&Msg);
}

uint16_t KA_BVTC::SetParams(void)
{
    uint16_t rc;
    tagMsg Msg;
    loadParam();
    for(int i = 0; i < count_n; i++) {
	if(data[i].Value.lnk.vlattr.at().getI(0, true) != TC_DISABLED) {
	    Msg.L = 0;
	    Msg.C = SetData;
	    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 1));
	    Msg.L += SerializeB(Msg.D + Msg.L, data[i].Period.lnk.vlattr.at().getI(0, true));
	    Msg.L += SerializeB(Msg.D + Msg.L, data[i].Count.lnk.vlattr.at().getI(0, true));
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

uint16_t KA_BVTC::RefreshParams(void)
{
    uint16_t rc;
    tagMsg Msg;
    for(int i = 1; i <= count_n; i++) {
	Msg.L = 0;
	Msg.C = AddrReq;
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i, 1));
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

uint16_t KA_BVTC::PostInit(void)
{
    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    for(int i = 0; i < count_n; i++) {
	Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ID, i + 1, 0));
	Msg.L += SerializeB(Msg.D + Msg.L, data[i].Value.lnk.vlattr.at().getI(0, true));
    }
    Msg.L += 3;
    return mPrm.owner().DoCmd(&Msg);
}

uint16_t KA_BVTC::RefreshData(void)
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

void KA_BVTC::loadIO(bool force)
{
    //Load links
    if(mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }	//Load/reload IO context only allow for stopped controllers for prevent throws
    for(int i = 0; i < count_n; i++) {
	loadLnk(data[i].Value.lnk);
	loadLnk(data[i].Count.lnk);
	loadLnk(data[i].Period.lnk);
    }
}

void KA_BVTC::saveIO(void)
{
    //Save links
    for(int i = 0; i < count_n; i++) {
	saveLnk(data[i].Value.lnk);
	saveLnk(data[i].Count.lnk);
	saveLnk(data[i].Period.lnk);
    }
}

void KA_BVTC::saveParam(void)
{
    for(int i = 0; i < count_n; i++) {
	saveVal(data[i].Value.lnk);
	saveVal(data[i].Count.lnk);
	saveVal(data[i].Period.lnk);
    }
}

void KA_BVTC::loadParam(void)
{
    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "load param");
    for(int i = 0; i < count_n; i++) {
	loadVal(data[i].Value.lnk);
	loadVal(data[i].Period.lnk);
	loadVal(data[i].Count.lnk);
    }
}

void KA_BVTC::tmHandler(void)
{
    for(int i = 0; i < count_n; i++) {
	UpdateParam8(data[i].Value, PackID(ID, (i + 1), 0), 1);
	if(with_params) {
	    UpdateParam28(data[i].Period, data[i].Count, PackID(ID, (i + 1), 1), 1);
	}

    }
    NeedInit = false;
}

uint16_t KA_BVTC::HandleEvent(int64_t tm, uint8_t * D)
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
	    for(int j = 1; j <= count_n; j++) {
		mPrm.vlAt(TSYS::strMess("TC_%d", j)).at().setI(D[j * 2 + 1], tm, true);
	    }
	    break;
	}
	break;
    default:
	switch(ft3ID.n) {
	case 0:
	    l = 4;
	    if(ft3ID.k > count_n) break;
	    mPrm.vlAt(TSYS::strMess("TC_%d", ft3ID.k)).at().setI(D[3], tm, true);
	    break;
	case 1:
	    l = 5;
	    if(with_params) {
		if(ft3ID.k > count_n) break;
		mPrm.vlAt(TSYS::strMess("Period_%d", ft3ID.k)).at().setI(D[3], tm, true);
		mPrm.vlAt(TSYS::strMess("Count_%d", ft3ID.k)).at().setI(D[4], tm, true);
	    }
	    break;
	}
	break;
    }
    return l;
}

uint8_t KA_BVTC::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);
//    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "ID %d ft3ID g%d k%d n%d ", ID, ft3ID.g, ft3ID.k, ft3ID.n);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
    if(ft3ID.k == 0) {
	switch(ft3ID.n) {
	case 0:
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
		out[i * 2] = data[i].Value.s;
		out[i * 2 + 1] = data[i].Value.vl;
	    }
	    l = count_n * 2;
	    break;
	}
    } else {
	if(ft3ID.k <= count_n) {
	    switch(ft3ID.n) {
	    case 0:
		out[0] = data[ft3ID.k - 1].Value.s;
		out[1] = data[ft3ID.k - 1].Value.vl;
		l = 2;
		break;
	    case 1:
		out[0] = data[ft3ID.k - 1].Period.s;
		out[1] = data[ft3ID.k - 1].Period.vl;
		out[2] = data[ft3ID.k - 1].Count.vl;
		l = 3;
		break;
	    }
	}
    }
    return l;
}

uint8_t KA_BVTC::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
//    if(mess_lev() == TMess::Debug) mPrm.mess_sys(TMess::Debug, "cmdSet k %d n %d", ft3ID.k, ft3ID.n);
    uint l = 0;
    if((ft3ID.k > 0) && (ft3ID.k <= count_n)) {
	switch(ft3ID.n) {
	case 0:
	    l = SetNew8Val(data[ft3ID.k - 1].Value, addr, prmID, req[2]);
	    break;
	case 1:
	    l = SetNew28Val(data[ft3ID.k - 1].Period, data[ft3ID.k - 1].Count, addr, prmID, req[2], req[3]);
	    break;
	}
    }
    return l;
}

uint16_t KA_BVTC::setVal(TVal &val)
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
    if((ft3ID.k >= 1) && (ft3ID.k <= count_n)) {
	switch(ft3ID.n) {
	case 0:
	    Msg.L += SerializeB(Msg.D + Msg.L, val.getI(0, true));
	    break;
	case 1:
	    Msg.L += SerializeB(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("Period_%d", ft3ID.k)).at().getI(0, true));
	    Msg.L += SerializeB(Msg.D + Msg.L, mPrm.vlAt(TSYS::strMess("Count_%d", ft3ID.k)).at().getI(0, true));
	    rc = 1;
	    break;
	}
    }
    if(Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }

    return rc;
}

//---------------------------------------------------------------------------

B_BVTC::B_BVTC(TMdPrm& prm, uint16_t id, uint16_t n, bool has_params) :
	DA(prm), count_n(n), ID(id), with_params(has_params)
{
    mTypeFT3 = GRS;
    blkID = 0x00;
    TFld * fld;
    mPrm.p_el.fldAdd(fld = new TFld("state", _("State"), TFld::Integer, TFld::NoWrite));
    fld->setReserve("0:0");
    for(int i = 0; i < count_n; i++) {
	AddChannel(i);
    }
    loadIO(true);
}

void B_BVTC::AddChannel(uint8_t iid)
{
    data.push_back(STCchannel(iid, this));
    AddAttr(data.back().Value.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("1:%d", iid / 8));
    if(with_params) {
	AddAttr(data.back().Mask.lnk, TFld::Integer, TVal::DirWrite, TSYS::strMess("2:%d", iid / 8));
    }
}

B_BVTC::~B_BVTC()
{
    data.clear();
}

string B_BVTC::getStatus(void)
{
    string rez;
    if(NeedInit) {
	rez = "20: ??????????";
    } else {
	rez = "0: ??????????";
    }
    return rez;

}

void B_BVTC::loadIO(bool force)
{
    //Load links
    if(mPrm.owner().startStat() && !force) {
	mPrm.modif(true);
	return;
    }	//Load/reload IO context only allow for stopped controllers for prevent throws
    for(int i = 0; i < count_n; i++) {
	loadLnk(data[i].Value.lnk);
	loadLnk(data[i].Mask.lnk);
    }
}

void B_BVTC::saveIO()
{
    //Save links
    for(int i = 0; i < count_n; i++) {
	saveLnk(data[i].Value.lnk);
	saveLnk(data[i].Mask.lnk);
    }
}

void B_BVTC::tmHandler(void)
{
    for(int i = 0; i < count_n; i++) {
	uint8_t tmpval;
	uint8_t g = i / 8;
	if(data[i].Mask.vl == 0) {
	    tmpval = data[i].Value.Get();
	    if(tmpval != data[i].Value.vl) {
		data[i].Value.Update(tmpval);
		uint8_t E[1] = { 0 };
		for(int j = 0; j < 8; j++) {
		    E[0] |= (data[g * 8 + j].Value.vl & 0x01) << j;
		}
		PushInBE(1, 1, PackID(ID, 1, i / 8), E);
	    }
	}
	if(with_params) {
	    tmpval = data[i].Mask.Get();
	    if(tmpval != data[i].Mask.vl) {
		data[i].Mask.s = 0;
		data[i].Mask.Update(tmpval);
		uint8_t E[2] = { 0, 0 };
		for(int j = 0; j < 8; j++) {
		    E[1] |= (data[g * 8 + j].Mask.vl & 0x01) << j;
		}
		PushInBE(1, 2, PackID(ID, 2, i / 8), E);
	    }
	}
    }
    NeedInit = false;
}

uint16_t B_BVTC::Task(uint16_t uc)
{
    tagMsg Msg;
    uint16_t rc = 0;
    switch(uc) {
    case TaskRefresh:
	Msg.L = 5;
	Msg.C = AddrReq;
	*((uint16_t *) Msg.D) = PackID(ID, 0, 0); //state
	if(mPrm.owner().DoCmd(&Msg)) {
	    if(Msg.C == GOOD3) {
		uint16_t nTC = (count_n / 8 + (count_n % 8 ? 1 : 0));
		Msg.L = 3 + nTC * 2;
		Msg.C = AddrReq;
		for(int i = 0; i < nTC; i++) {
		    *((uint16_t *) (Msg.D + i * 2)) = PackID(ID, 1, i); //TC Value
		}
		if(mPrm.owner().DoCmd(&Msg)) {
		    if(with_params) {
			Msg.L = 3 + nTC * 2;
			Msg.C = AddrReq;
			for(int i = 0; i < nTC; i++) {
			    *((uint16_t *) (Msg.D + i * 2)) = PackID(ID, 2, i); //?????????? ??C
			}
			if(mPrm.owner().DoCmd(&Msg)) {
			    rc = 1;
			}
		    } else {
			rc = 1;
		    }
		}
	    }
	}
	if(rc) NeedInit = false;
	break;
    }
    return rc;
}

uint16_t B_BVTC::HandleEvent(int64_t tm, uint8_t * D)
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
	    l = 3 + count_n / 4;
	    for(int j = 1; j <= count_n; j++) {
		mPrm.vlAt(TSYS::strMess("TC_%d", j)).at().setB((D[((j - 1) >> 3) + 3] >> (j % 8)) & 1, tm, true);
		if(with_params) {
		    mPrm.vlAt(TSYS::strMess("Mask_%d", j)).at().setB((D[((j - 1) >> 3) + 3 + count_n / 8] >> (j % 8)) & 1, tm, true);
		}
	    }
	    break;
	}
	break;
    case 1:
	l = 3;
	for(int i = 0; i < 8; i++) {
	    if((1 + (ft3ID.n << 3) + i) > count_n) break;
	    mPrm.vlAt(TSYS::strMess("TC_%d", 1 + (ft3ID.n << 3) + i)).at().setB((D[2] >> i) & 1, tm, true);
	}
	break;
    case 2:
	l = 4;
	if(with_params) {
	    for(int i = 0; i < 8; i++) {
		if((1 + (ft3ID.n << 3) + i) > count_n) break;
		mPrm.vlAt(TSYS::strMess("Mask_%d", 1 + (ft3ID.n << 3) + i)).at().setB((D[3] >> i) & 1, tm, true);
	    }
	}
	break;
    }
    return l;
}

uint8_t B_BVTC::cmdGet(uint16_t prmID, uint8_t * out)
{
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;
    uint16_t nTC = (count_n / 8 + (count_n % 8 ? 1 : 0));
    switch(ft3ID.k) {
    case 0:
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
	    for(uint8_t i = 0; i < nTC; i++) {
		out[i + 1] = 0;
		for(uint8_t j = i * 8; j < (i + 1) * 8; j++)
		    out[i + 1] |= (data[j].Value.vl & 0x01) << (j % 8);
		l++;
	    }
	    //mask;
	    for(uint8_t i = 0; i < nTC; i++) {
		out[i + nTC + 1] = 0;
		for(uint8_t j = i * 8; j < (i + 1) * 8; j++)
		    out[i * 2 + 2] |= (data[j].Mask.vl & 0x01) << (j % 8);
		l++;
	    }
	    break;
	}
	break;

    case 1:
	//value
	out[0] = 0;
	if(ft3ID.n < nTC) {
	    for(uint8_t j = ft3ID.n * 8; j < (ft3ID.n + 1) * 8; j++) {
		out[0] |= (data[j].Value.vl & 0x01) << (j % 8);
	    }
	    l = 1;
	}
	break;
    case 2:
	//mask
	out[0] = 0;
	out[1] = 0;
	if(ft3ID.n < nTC) {
	    for(uint8_t j = ft3ID.n * 8; j < (ft3ID.n + 1) * 8; j++) {
		out[0] = data[j].Mask.s;
		out[1] |= (data[j].Mask.vl & 0x01) << (j % 8);
	    }
	    l = 2;
	}
	break;
    }
    return l;
}

uint8_t B_BVTC::cmdSet(uint8_t * req, uint8_t addr)
{
    uint16_t prmID = TSYS::getUnalign16(req);
    FT3ID ft3ID = UnpackID(prmID);
    if(ft3ID.g != ID) return 0;
    uint l = 0;

    uint16_t nTC = (count_n / 8 + (count_n % 8 ? 1 : 0));
    switch(ft3ID.k) {
    case 2:
	if(ft3ID.n < nTC) {
	    uint8_t newMask = req[2];
	    for(uint8_t i = ft3ID.n * 8; i < (ft3ID.n + 1) * 8; i++) {
		data[i].Mask.s = addr;
		if(data[i].Mask.lnk.Connected()) {
		    data[i].Mask.Set((uint8_t) (newMask & 0x01));
		    newMask = newMask >> 1;
		    l = 3;
		} else {
		    l = 0;
		    break;
		}

	    }
	    uint8_t E[2] = { addr, req[2] };
	    mPrm.owner().PushInBE(1, 2, prmID, E);
	}
	break;
    }
    return l;
}

uint16_t B_BVTC::setVal(TVal &val)
{
    int off = 0;
    FT3ID ft3ID;
    ft3ID.k = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.n = s2i(TSYS::strParse(val.fld().reserve(), 0, ":", &off));
    ft3ID.g = ID;

    uint16_t st = ft3ID.n * 8 + 1;
    uint16_t en = (ft3ID.n + 1) * 8;
    if(en > count_n) en = count_n;

    tagMsg Msg;
    Msg.L = 0;
    Msg.C = SetData;
    Msg.L += SerializeUi16(Msg.D + Msg.L, PackID(ft3ID));
    uint8_t mask = 0;
    for(int i = st; i <= en; i++) {
	mask |= ((mPrm.vlAt(TSYS::strMess("Mask_%d", i)).at().getB(0, true)) << ((i - 1) % 8));
    }
    Msg.L += SerializeB(Msg.D + Msg.L, val.getI(0, true));
    if(Msg.L > 2) {
	Msg.L += 3;
	mPrm.owner().DoCmd(&Msg);
    }
    return 0;
}

//---------------------------------------------------------------------------
