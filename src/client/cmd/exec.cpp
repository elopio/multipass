/*
 * Copyright (C) 2017 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alberto Aguirre <alberto.aguirre@canonical.com>
 *
 */

#include "exec.h"

#include <multipass/cli/argparser.h>
#include <multipass/ssh/ssh_client.h>

namespace mp = multipass;
namespace cmd = multipass::cmd;
using RpcMethod = mp::Rpc::Stub;

mp::ReturnCode cmd::Exec::run(mp::ArgParser* parser)
{
    auto ret = parse_args(parser);
    if (ret != ParseCode::Ok)
    {
        return parser->returnCodeFrom(ret);
    }

    auto on_success = [&parser](mp::SSHInfoReply& reply) {
        auto host = reply.host();
        auto port = reply.port();

        // TODO: mainly for testing - need a better way to test parsing
        if (port == 0)
            return ReturnCode::Ok;

        auto priv_key_blob = reply.priv_key_base64();

        std::vector<std::string> args;
        for (int i = 1; i < parser->positionalArguments().size(); ++i)
            args.push_back(parser->positionalArguments().at(i).toStdString());

        mp::SSHClient ssh_client{host, port, priv_key_blob};
        return static_cast<mp::ReturnCode>(ssh_client.exec(args));
    };

    auto on_failure = [this](grpc::Status& status) {
        cerr << "exec failed: " << status.error_message() << "\n";
        return ReturnCode::CommandFail;
    };

    return dispatch(&RpcMethod::ssh_info, request, on_success, on_failure);
}

std::string cmd::Exec::name() const
{
    return "exec";
}

QString cmd::Exec::short_help() const
{
    return QStringLiteral("Run a command on an instance");
}

QString cmd::Exec::description() const
{
    return QStringLiteral("Run a command on an instance");
}

mp::ParseCode cmd::Exec::parse_args(mp::ArgParser* parser)
{
    parser->addPositionalArgument("name", "Name of instance to execute the command on", "<name>");
    parser->addPositionalArgument("command", "Command to execute on the instance", "[--] <command>");

    auto status = parser->commandParse(this);

    if (status != ParseCode::Ok)
    {
        return status;
    }

    if (parser->positionalArguments().count() < 2)
    {
        cerr << "Wrong number of arguments" << std::endl;
        status = ParseCode::CommandLineError;
    }
    else
    {
        request.set_instance_name(parser->positionalArguments().first().toStdString());
    }

    return status;
}
