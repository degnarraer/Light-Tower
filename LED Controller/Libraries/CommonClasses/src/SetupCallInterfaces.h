/*
    Light Tower by Rob Shockency
    Copyright (C) 2021 Rob Shockency degnarraer@yahoo.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version of the License, or
    (at your option) any later version. 3

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#include <vector>
#include "Streaming.h"

class SetupCalleeInterface
{
	public:
		SetupCalleeInterface(){}
		virtual ~SetupCalleeInterface(){}
		virtual void Setup() = 0;
};
class SetupCallerInterface
{
	public:
		SetupCallerInterface(){}
		virtual ~SetupCallerInterface(){}
		virtual void RegisterForSetupCall(SetupCalleeInterface* Callee);
		virtual void DeRegisterForSetupCall(SetupCalleeInterface* Callee);
	protected:
		virtual void SetupAllSetupCallees();
	private:
		std::vector<SetupCalleeInterface*> m_SetupCallees = std::vector<SetupCalleeInterface*>();
};