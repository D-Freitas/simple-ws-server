#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <stdio.h>

using tcp = boost::asio::ip::tcp;

void open_socket(const boost::asio::ip::address address, const short unsigned int port)
{
  boost::asio::io_context ioc{1};
  tcp::acceptor acceptor{ioc, {address, port}};

  while (true)
  {
    tcp::socket socket(ioc);
    acceptor.accept(socket);
    std::cout << "socket accepted" << std::endl;

    std::thread{[q {std::move(socket)}]() {
      boost::beast::websocket::stream<tcp::socket> ws {std::move(const_cast<tcp::socket&>(q))};

      ws.set_option(boost::beast::websocket::stream_base::decorator(
      [](boost::beast::websocket::response_type& res){
        res.set(boost::beast::http::field::server, "boost ws server");
      }));

      ws.accept();

      while (true)
      {
        try
        {
          boost::beast::flat_buffer buffer;
          ws.read(buffer);
          auto out = boost::beast::buffers_to_string(buffer.cdata());
          std::cout << out << std::endl;

          ws.write(buffer.data());
        }
        catch(boost::beast::system_error const& error)
        {
          if (error.code() != boost::beast::websocket::error::closed)
          {
            std::cout << error.code().message() << std::endl;
            break;
          }
        }
      }
    }}.detach();
  }
}

int main()
{
  auto const address = boost::asio::ip::make_address("127.0.0.1");
  const std::string port_number = "8083";
  
  auto const port = static_cast<unsigned short>(atoi(port_number.c_str()));

  open_socket(address, port);
  return 0;
}
