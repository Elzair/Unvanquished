/*
===========================================================================

Daemon GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

This file is part of the Daemon GPL Source Code (Daemon Source Code).

Daemon Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Daemon Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Daemon Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Daemon Source Code is also subject to certain additional terms.
You should have received a copy of these additional terms immediately following the
terms and conditions of the GNU General Public License which accompanied the Daemon
Source Code.  If not, please request a copy in writing from id Software at the address
below.

If you have questions concerning this license or the applicable additional terms, you
may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville,
Maryland 20850 USA.

===========================================================================
*/

#include "CommandSystem.h"

#include <unordered_map>
#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"

namespace Cmd {

    std::unordered_map<std::string, const CmdBase*> commands;

    //TODO: remove the need for this
    Args currentArgs;
    Args oldArgs;

    void AddCommand(const std::string& name, const CmdBase* cmd) {
        if (commands.count(name)) {
			Com_Printf(_( "Cmd::AddCommand: %s already defined\n"), name.c_str() );
			return;
        }

        commands[name] = cmd;
    }

    void RemoveCommand(const std::string& name) {
        commands.erase(name);
    }

    void RemoveFlaggedCommands(cmdFlags_t flag) {
        for (auto it = commands.cbegin(); it != commands.cend();) {
            const CmdBase* cmd = it->second;

            if (cmd->GetFlags() & flag) {
                commands.erase(it ++);
            } else {
                ++ it;
            }
        }
    }

    bool CommandExists(const std::string& name) {
        return commands.count(name);
    }

    void ExecuteCommandString(const std::string& command) {
        Args args(command);
        currentArgs = args;

        if (args.Argc() == 0) {
            return;
        }

        const std::string& cmdName = args.Argv(0);
        if (commands.count(cmdName)) {
            commands[cmdName]->Run(args);
            return;
        }

        //TODO: remove that and add default command handlers or something
        // check cvars
        if (Cvar_Command()) {
            return;
        }

        // check client game commands
        if (com_cl_running && com_cl_running->integer && CL_GameCommand()) {
            return;
        }

        // check server game commands
        if (com_sv_running && com_sv_running->integer && SV_GameCommand()) {
            return;
        }

        // check ui commands
        if (com_cl_running && com_cl_running->integer && UI_GameCommand()) {
            return;
        }

        // send it as a server command if we are connected
        // (cvars are expanded locally)
        CL_ForwardCommandToServer( args.OriginalArgs(0).c_str() );
    }

    std::vector<std::string> CommandNames() {
        std::vector<std::string> res;
        for (auto entry: commands) {
            res.push_back(entry.first);
        }
        return res;
    }

    std::vector<std::string> CompleteArgument(const std::string& command, int pos) {
        Args args(command);
        int argNum = args.ArgNumber(pos);
        const std::string& cmdName = args.Argv(0);

        if (!commands.count(cmdName)) {
            return {};
        }

        const CmdBase* cmd = commands[cmdName];
        return cmd->Complete(argNum, args);
    }

    const Args& GetCurrentArgs() {
        return currentArgs;
    }

    void SetCurrentArgs(const Args& args) {
        currentArgs = args;
    }

    void SaveArgs() {
        oldArgs = currentArgs;
    }

    void LoadArgs() {
        currentArgs = oldArgs;
    }
}
