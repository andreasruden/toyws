#include "toyws/request_handler.hpp"

#include "toyws/http_request.hpp"
#include "toyws/io_service_impl.hpp"

auto toyws::RequestHandler::OnAccept(IoService<RequestHandler>* service,
                                     Socket listenSock, Client* client)
    -> void {
  service->AsyncAccept(listenSock);
  service->AsyncRead(client->IoServiceSlot());
}

auto toyws::RequestHandler::OnRead(IoService<RequestHandler>* service,
                                   Client* client) -> void {
  // TODO: Handle HttpRequest::Read returning false -> Read more!
  HttpRequest request;
  request.Read(client->Buffer().data(), client->BufferContentSize());
  auto response = service->Instance()->HandleRequest(request);

  // TODO: Handle HttpRequest::Write returning false -> Write multiple times!
  auto written =
      response.Write(client->Buffer().data(), client->Buffer().size());
  client->SetBufferContentSize(written.second);

  service->AsyncWrite(client->IoServiceSlot());
}

auto toyws::RequestHandler::OnWrite(IoService<RequestHandler>* service,
                                    Client* client) -> void {
  // TODO: Handle multiple writes
  service->Close(client);
}

namespace toyws {
template class IoService<RequestHandler>;
}  // namespace toyws
