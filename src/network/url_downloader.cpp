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
 * Authored by: Chris Townsend <christopher.townsend@canonical.com>
 *              Alberto Aguirre <alberto.aguirre@canonical.com>
 *
 */

#include <multipass/url_downloader.h>

#include <QDir>
#include <QEventLoop>
#include <QFile>
#include <QNetworkReply>
#include <QUrl>

#include <memory>

namespace mp = multipass;

namespace
{
template <typename Action, typename ErrorAction>
QByteArray download(QNetworkAccessManager& manager, QUrl const& url, Action&& action, ErrorAction&& on_error)
{
    QEventLoop event_loop;

    QNetworkRequest request{url};
    request.setRawHeader("Connection", "Keep-Alive");
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);

    auto reply = std::unique_ptr<QNetworkReply>(manager.get(request));

    QObject::connect(reply.get(), &QNetworkReply::finished, &event_loop, &QEventLoop::quit);
    QObject::connect(reply.get(), &QNetworkReply::downloadProgress, [&](qint64 bytes_received, qint64 bytes_total) {
        action(reply.get(), bytes_received, bytes_total);
    });

    event_loop.exec();
    if (reply->error() != QNetworkReply::NoError)
    {
        on_error();
        throw std::runtime_error(reply->errorString().toStdString());
    }
    return reply->readAll();
}
}

mp::URLDownloader::URLDownloader(const mp::Path& cache_dir, std::ostream& cerr) : cerr{cerr}
{
    network_cache.setCacheDirectory(QDir(cache_dir).filePath("network-cache"));
    manager.setCache(&network_cache);
}

void mp::URLDownloader::download_to(const QUrl& url, const QString& file_name, int64_t size, const int download_type,
                                    const mp::ProgressMonitor& monitor)
{
    QFile file{file_name};
    file.open(QIODevice::ReadWrite | QIODevice::Truncate);

    auto progress_monitor = [this, &file, &monitor, download_type, size](QNetworkReply* reply, qint64 bytes_received,
                                                                         qint64 bytes_total) {
        if (bytes_total == -1)
            bytes_total = size;
        auto progress = (size < 0) ? size : (100 * bytes_received + bytes_total / 2) / bytes_total;
        if (file.write(reply->readAll()) < 0)
        {
            cerr << "Error writing image: " << file.errorString().toStdString() << std::endl;
            reply->abort();
        }
        if (!monitor(download_type, progress))
        {
            reply->abort();
        }
    };

    auto on_error = [&file]() { file.remove(); };

    ::download(manager, url, progress_monitor, on_error);
}

QByteArray mp::URLDownloader::download(const QUrl& url)
{
    return ::download(manager, url, [](QNetworkReply*, qint64, qint64) {}, [] {});
}
