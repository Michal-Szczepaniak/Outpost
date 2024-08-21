/*

This file is part of Outpost.
Copyright 2023, Michał Szczepaniak <m.szczepaniak.000@gmail.com>

Outpost is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Outpost is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Yottagram. If not, see <http://www.gnu.org/licenses/>.

*/

#ifndef ENDPOINTS_H
#define ENDPOINTS_H

#include <string>

namespace Endpoints {

static const std::string SMS_SEND_CODE = "https://api-inmobile-pl.easypack24.net/v1/account";
static const std::string SMS_CONFIRM_CODE = "https://api-inmobile-pl.easypack24.net/v1/account/verification";
static const std::string REFRESH_TOKEN = "https://api-inmobile-pl.easypack24.net/v1/authenticate";
static const std::string LOGOUT = "https://api-inmobile-pl.easypack24.net/v1/logout";
static const std::string PARCELS = "https://api-inmobile-pl.easypack24.net/v4/parcels/tracked";
static const std::string SENT = "https://api-inmobile-pl.easypack24.net/v2/parcels/sent";
static const std::string RETURNS = "https://api-inmobile-pl.easypack24.net/v1/returns/parcels";
static const std::string COMPARTMENT_OPEN = "https://api-inmobile-pl.easypack24.net/v1/collect/compartment/open";
static const std::string OBSERVED_PARCEL = "https://api-inmobile-pl.easypack24.net/v1/observedParcel";

}

#endif // ENDPOINTS_H
