/******************************************************************************
 * Icinga 2                                                                   *
 * Copyright (C) 2012-2014 Icinga Development Team (http://www.icinga.org)    *
 *                                                                            *
 * This program is free software; you can redistribute it and/or              *
 * modify it under the terms of the GNU General Public License                *
 * as published by the Free Software Foundation; either version 2             *
 * of the License, or (at your option) any later version.                     *
 *                                                                            *
 * This program is distributed in the hope that it will be useful,            *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of             *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the              *
 * GNU General Public License for more details.                               *
 *                                                                            *
 * You should have received a copy of the GNU General Public License          *
 * along with this program; if not, write to the Free Software Foundation     *
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.             *
 ******************************************************************************/

#include "icinga/pluginutility.hpp"
#include "icinga/macroprocessor.hpp"
#include "icinga/perfdatavalue.hpp"
#include "base/logger_fwd.hpp"
#include "base/utility.hpp"
#include "base/convert.hpp"
#include "base/process.hpp"
#include "base/objectlock.hpp"
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/foreach.hpp>

using namespace icinga;

struct CommandArgument
{
	int Order;
	bool SkipKey;
	bool SkipValue;
	String Key;
	String Value;

	CommandArgument(void)
		: Order(0), SkipKey(false), SkipValue(false)
	{ }

	bool operator<(const CommandArgument& rhs) const
	{
		return Order < rhs.Order;
	}
};

void PluginUtility::ExecuteCommand(const Command::Ptr& commandObj, const Checkable::Ptr& checkable,
    const CheckResult::Ptr& cr, const MacroProcessor::ResolverList& macroResolvers,
    const boost::function<void(const Value& commandLine, const ProcessResult&)>& callback)
{
	Value raw_command = commandObj->GetCommandLine();
	Dictionary::Ptr raw_arguments = commandObj->GetArguments();

	Value command;
	if (!raw_arguments || raw_command.IsObjectType<Array>())
		command = MacroProcessor::ResolveMacros(raw_command, macroResolvers, cr, NULL, Utility::EscapeShellArg);
	else {
		Array::Ptr arr = make_shared<Array>();
		arr->Add(raw_command);
		command = arr;
	}

	if (raw_arguments) {
		std::vector<CommandArgument> args;

		ObjectLock olock(raw_arguments);
		BOOST_FOREACH(const Dictionary::Pair& kv, raw_arguments) {
			const Value& arginfo = kv.second;

			CommandArgument arg;
			arg.Key = kv.first;

			bool required = false;
			String argval;

			if (arginfo.IsObjectType<Dictionary>()) {
				Dictionary::Ptr argdict = arginfo;
				argval = argdict->Get("value");
				if (argdict->Contains("required"))
					required = argdict->Get("required");
				arg.SkipKey = argdict->Get("skip_key");
				arg.Order = argdict->Get("order");

				String set_if = argdict->Get("set_if");

				if (!set_if.IsEmpty()) {
					String missingMacro;
					String set_if_resolved = MacroProcessor::ResolveMacros(set_if, macroResolvers,
						cr, &missingMacro);

					if (!missingMacro.IsEmpty())
						continue;

					try {
						if (!Convert::ToLong(set_if_resolved))
							continue;
					} catch (const std::exception& ex) {
						/* tried to convert a string */
						Log(LogWarning, "PluginUtility", "Error evaluating set_if value '" + set_if_resolved + "': " + ex.what());
						continue;
					}
				}
			}
			else
				argval = arginfo;

			if (argval.IsEmpty())
				arg.SkipValue = true;

			String missingMacro;
			arg.Value = MacroProcessor::ResolveMacros(argval, macroResolvers,
			    cr, &missingMacro);

			if (!missingMacro.IsEmpty()) {
				if (required) {
					String message = "Non-optional macro '" + missingMacro + "' used in argument '" +
					    arg.Key + "' is missing while executing command '" + commandObj->GetName() +
					    "' for object '" + checkable->GetName() + "'";
					Log(LogWarning, "PluginUtility", message);

					if (callback) {
						ProcessResult pr;
						pr.PID = -1;
						pr.ExecutionStart = Utility::GetTime();
						pr.ExecutionStart = pr.ExecutionStart;
						pr.ExitStatus = 3; /* Unknown */
						pr.Output = message;
						callback(Empty, pr);
					}

					return;
				}

				continue;
			}

			args.push_back(arg);
		}

		std::sort(args.begin(), args.end());

		Array::Ptr command_arr = command;
		BOOST_FOREACH(const CommandArgument& arg, args) {
			if (!arg.SkipKey)
				command_arr->Add(arg.Key);

			if (!arg.SkipValue)
				command_arr->Add(arg.Value);
		}
	}

	Dictionary::Ptr envMacros = make_shared<Dictionary>();

	Dictionary::Ptr env = commandObj->GetEnv();

	if (env) {
		ObjectLock olock(env);
		BOOST_FOREACH(const Dictionary::Pair& kv, env) {
			String name = kv.second;

			Value value = MacroProcessor::ResolveMacros(name, macroResolvers, cr);

			envMacros->Set(kv.first, value);
		}
	}

	Process::Ptr process = make_shared<Process>(Process::PrepareCommand(command), envMacros);
	process->SetTimeout(commandObj->GetTimeout());
	process->Run(boost::bind(callback, command, _1));
}

ServiceState PluginUtility::ExitStatusToState(int exitStatus)
{
	switch (exitStatus) {
		case 0:
			return ServiceOK;
		case 1:
			return ServiceWarning;
		case 2:
			return ServiceCritical;
		default:
			return ServiceUnknown;
	}
}

std::pair<String, String> PluginUtility::ParseCheckOutput(const String& output)
{
	String text;
	String perfdata;

	std::vector<String> lines;
	boost::algorithm::split(lines, output, boost::is_any_of("\r\n"));

	BOOST_FOREACH (const String& line, lines) {
		size_t delim = line.FindFirstOf("|");

		if (!text.IsEmpty())
			text += "\n";

		if (delim != String::NPos) {
			text += line.SubStr(0, delim);

			if (!perfdata.IsEmpty())
				perfdata += " ";

			perfdata += line.SubStr(delim + 1, line.GetLength());
		} else {
			text += line;
		}
	}

	boost::algorithm::trim(perfdata);

	return std::make_pair(text, perfdata);
}

Array::Ptr PluginUtility::SplitPerfdata(const String& perfdata)
{
	Array::Ptr result = make_shared<Array>();

	size_t begin = 0;
	String multi_prefix;

	for (;;) {
		size_t eqp = perfdata.FindFirstOf('=', begin);

		if (eqp == String::NPos)
			break;

		String label = perfdata.SubStr(begin, eqp - begin);

		if (label.GetLength() > 2 && label[0] == '\'' && label[label.GetLength() - 1] == '\'')
			label = label.SubStr(1, label.GetLength() - 2);

		size_t multi_index = label.RFind("::");

		if (multi_index != String::NPos)
			multi_prefix = "";

		size_t spq = perfdata.FindFirstOf(' ', eqp);

		if (spq == String::NPos)
			spq = perfdata.GetLength();

		String value = perfdata.SubStr(eqp + 1, spq - eqp - 1);

		if (!multi_prefix.IsEmpty())
			label = multi_prefix + "::" + label;

		String pdv;
		if (label.FindFirstOf(" ") != String::NPos)
			pdv = "'" + label + "'=" + value;
		else
			pdv = label + "=" + value;

		result->Add(pdv);

		if (multi_index != String::NPos)
			multi_prefix = label.SubStr(0, multi_index);

		begin = spq + 1;
	}

	return result;
}

String PluginUtility::FormatPerfdata(const Array::Ptr& perfdata)
{
	if (!perfdata)
		return "";

	std::ostringstream result;

	ObjectLock olock(perfdata);

	bool first = true;
	BOOST_FOREACH(const Value& pdv, perfdata) {
		if (!first)
			result << " ";
		else
			first = false;

		if (pdv.IsObjectType<PerfdataValue>())
			result << static_cast<PerfdataValue::Ptr>(pdv)->Format();
		else
			result << pdv;
	}

	return result.str();
}
