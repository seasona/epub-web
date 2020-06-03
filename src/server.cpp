#include "server.h"
#include "spdlog/spdlog.h"

namespace Jebook {

Server::Server(const char* address, const char* port, const char* doc_root)
    : doc_root_(doc_root),
      io_context_(1),
      acceptor_(io_context_),
      connection_manager_() {
    asio::ip::tcp::resolver resolver(io_context_);
    asio::ip::tcp::endpoint endpoint =
        *(resolver.resolve(address, port).begin());

    acceptor_.open(endpoint.protocol());
    acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
    acceptor_.bind(endpoint);
    acceptor_.listen();

    doAccept();
}

void Server::doAccept() {
    acceptor_.async_accept([this](asio::error_code ec,
                                  asio::ip::tcp::socket socket) {
        // TODO check
        if (!acceptor_.is_open()) {
            spdlog::error("The acceptor is not open.");
            return;
        }

        if (!ec) {
            spdlog::info("Accepted a socket.");
            connection_manager_.start(std::make_shared<Connection>(
                std::move(socket), connection_manager_, doc_root_, templates_));
        }

        doAccept();
    });
}

void Server::setTemplatePath(std::string template_path) {
    templates_.setTemplatePath(template_path);
}

void Server::run() {
    spdlog::info("The server is running.");
    // TODO different path separator in win
    std::string ncx_path = std::string(doc_root_) + "/toc.ncx";
    templates_.setNcxPath(ncx_path);
    // convert directory xml to json first before server listen
    templates_.convert();
    io_context_.run();
}

}  // namespace Jebook