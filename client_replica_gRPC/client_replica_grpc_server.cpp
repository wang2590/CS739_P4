#include "client_replica_grpc_server.h"

#include <errno.h>
#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <signal.h>

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "client_replica.grpc.pb.h"

using client_replica::ReplyReq;
using common::Empty;
using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

using namespace std;
using namespace client_replica;
using grpc::StatusCode;

ClientReplicaGrpcServiceImpl::ClientReplicaGrpcServiceImpl(ReplicaState* state)
    : state_(state) {}

Status ClientReplicaGrpcServiceImpl::Request(ServerContext* context,
                                             const SignedMessage* request,
                                             Empty* reply) {
  if (state_->replica_id == state_->primary) {
    RequestCmd request_cmd;
    if (!request_cmd.ParseFromString(request->message())) {
      return Status(StatusCode::PERMISSION_DENIED,
                    "Failed to parse RequestCmd");
    }

    RsaPtr rsa = CreateRsa(request_cmd.c(), true);
    if (!VerifyMessage(request->message(), request->signature(), rsa.get())) {
      return Status(StatusCode::PERMISSION_DENIED, "Wrong request type!");
    }

    std::string digest = Sha256Sum(request->message());
    int sequence_n = 0;
    {
      const std::lock_guard<std::mutex> lock(state_->operation_history_lock);
      sequence_n = state_->operation_history.size();
      state_->operation_history.emplace_back(request_cmd, digest);
    }
    for (int i = 0; i < 3; ++i) {
      auto& client = state_->replica_clients[i];
      if (client == nullptr) continue;
      client->ReplicaPrePrepareClient(state_->view, sequence_n, *request,
                                      digest);
    }

    const SignedMessage fake_request = CreateFakeRequest(request_cmd);
    digest = Sha256Sum(fake_request.message());
    for (int i = 3; i < (int)state_->replica_clients.size(); ++i) {
      auto& client = state_->replica_clients[i];
      if (client == nullptr) continue;
      client->ReplicaPrePrepareClient(state_->view, sequence_n, fake_request,
                                      digest);
    }
  } else {
    // TODO: relay the request
    return Status(StatusCode::UNIMPLEMENTED,
                  "Only primary accepts the request.");
  }

  // reply Empty -> nothing
  return Status::OK;
}

Status ClientReplicaGrpcServiceImpl::Reply(
    ServerContext* context, const ReplyReq* request,
    ServerWriter<SignedMessage>* reply_writer) {
  // unsure about the client id where it goes
  const string client_id = request->client_id();
  // ReplyCmd Comsumer
  while (1) {                                                  // infinite loop
    auto time_out = std::chrono::system_clock::now() + 9999s;  // scary ):
    ReplyCmd result;
    if (state_->replies.do_get(time_out, result) != 0) continue;
    SignedMessage* reply = new SignedMessage();
    if (SignMessage(result, state_->private_key.get(), reply) < 0)
      return Status(StatusCode::PERMISSION_DENIED, "Wrong request type!");
    reply_writer->Write(*reply);
  }
  return Status::OK;
}

SignedMessage ClientReplicaGrpcServiceImpl::CreateFakeRequest(
    RequestCmd request_cmd) {
  const std::string fake_client_priv = R"(-----BEGIN RSA PRIVATE KEY-----
MIIEowIBAAKCAQEApkIlDITYrPCWx3BpJOTZN2MZ9h+lXpXqiULCaeMi0zcjN7xp
TL2IDZE0+Q92jRQ/sFU1MVfKr5AgU42voJOu0sQdBJ7crgkyi37urUHX3wkITT2Z
fLH0S9VgVQ5uYtvD8W5iXylLAr+1YVCdXsEC9G3G5SmhS76Fj7Z9tkEeU71cMBZz
7xY0PDmc0Yd2RlOuiLSKvEio1GG/j26FzvwFgOFKZSCY56Q+CJIQX8Il9zfch+Ve
OfHWMdTO3CmNcRF9SYYMINDK1YYqB13Ut6iJp78AWLbXAtmMRu4gsBTc471ypNAn
Qq5L6IwjJqqI2sIE2unBem6SOGCT3JWnbWFt+QIDAQABAoIBAHQwNYQSMvppCHZI
cLJ/2+2BLFt8sizvyAA0z3kAaw4AsnP5D/25VqNknwiPbeIaDIhGg+3/1H3s0yqR
EfO+Qaa5ty3Ld52If/JMurAKG7qarVqNef5Svs9gRvoqI6qiusS2x7iJOwqovzFL
DS9y44W+UglElpxAgUZ12lTMHU2XH525YqhSeEgzbXW6QrJVzWz9CihXTBnK61XF
eZ2UaCY9Z5Zf8De52NJeZXSa3iSxwg7uUaglL/jbFAfsRj3YRlm8d2fp7Of2vtZs
5pnEBMGO650nGV+dSHKEMzGn7u0lwCuhGS7fQtGmMHd+wnl86zIY0nWJkYm2syk2
oNmz7KECgYEA1PfY0ZYaRXKDpvCGK+XAVtO04EU4iQPc4zYkUFN8wQn5RqUuXgDJ
UbWldCqofk3mET06AvsSo+jBw4dFUnPuCUQNwravdFftMdoSeDGERAi2S3PUVRD/
sVT6MWujnr1q4vJeBq1ATgqyxF+WENi91/btfF6D2rle0f2l3JHLR8MCgYEAx9on
AXMGZJmexA8E/+YvROkfu21Z+HVNsnIlxMfWs0GeSjOKRdvu0ZrVC7Z2Vg6OtFkz
M2l+KWXaw5cR2eXwaJuFhDEcFEFjoThGDovlOv4JCRJkA4oSgqHANfm/t2Q6bfkt
/2eMY6KvGgRj36iWXvppFWf0311Vkqase6nhU5MCgYEAgEwDUVek4ft3tkUl/yH8
uhH14DmzMk+ibTq7q6xv9ncEtftisHy2y0OrtgzQzq4tEGubHBqXAymAOUOvjlmK
qAhYZdnaXzNtEGA4SjJCZR3+e/v+iXShJPp1cvpOap3f23J5FHqgx6MISObwp5t6
2rTGIutpcYXEyqqMlhE6rjMCgYBMpR98cM6s2jWHX3M9aNwMA5QF34Q3w1UO8LQH
adSvXqUENAv9isFmZYj7OvjbPC4CgIsgVn7qBYQGqvNPq92tYKet8N+h7Qn2lIxZ
m9FH9JEmh4UPzm4W50295ZBfFvNJpdy8LMwskgGxaqma5x70Qfj3Gq1Q1Iyy93UC
4D3oPQKBgA56MpUeOmNE6ZTTvjK5pVTf2CydyVVX3ER0uGI3oJ5UJkpquU6RGFyA
dxZG0hbTLJh6XIRdR5O1QFR6bcgJ/sghKQtQTvKqkH0VFqIvtI5jbJMZFUcYYaoW
dIWkHwzn7+jeyCPz5OCYHgH6moIB8hUx3i5ffdi5kM1F3OjfBuC4
-----END RSA PRIVATE KEY-----
)";
  const std::string fake_client_pub = R"(-----BEGIN PUBLIC KEY-----
MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEApkIlDITYrPCWx3BpJOTZ
N2MZ9h+lXpXqiULCaeMi0zcjN7xpTL2IDZE0+Q92jRQ/sFU1MVfKr5AgU42voJOu
0sQdBJ7crgkyi37urUHX3wkITT2ZfLH0S9VgVQ5uYtvD8W5iXylLAr+1YVCdXsEC
9G3G5SmhS76Fj7Z9tkEeU71cMBZz7xY0PDmc0Yd2RlOuiLSKvEio1GG/j26FzvwF
gOFKZSCY56Q+CJIQX8Il9zfch+VeOfHWMdTO3CmNcRF9SYYMINDK1YYqB13Ut6iJ
p78AWLbXAtmMRu4gsBTc471ypNAnQq5L6IwjJqqI2sIE2unBem6SOGCT3JWnbWFt
+QIDAQAB
-----END PUBLIC KEY-----
)";

  request_cmd.set_c(fake_client_pub);
  if (request_cmd.mutable_o()->has_read()) {
    request_cmd.mutable_o()->mutable_read()->set_offset(8192);
  } else {
    request_cmd.mutable_o()->mutable_write()->set_data("fake request");
  }

  RsaPtr rsa = CreateRsa(fake_client_priv, false);
  SignedMessage signed_message;
  assert(SignMessage(request_cmd, rsa.get(), &signed_message) == 0);
  return signed_message;
}
