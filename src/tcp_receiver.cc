#include "tcp_receiver.hh"

using namespace std;

void TCPReceiver::receive( TCPSenderMessage message )
{
  if ( message.RST ) {
    reassembler_.reader().set_error();
    return;
  }
  if ( message.SYN ) {
    isn_ = message.seqno;
  }
  if ( isn_ ) {
    uint64_t stream_index
      = message.SYN ? 0 : message.seqno.unwrap( isn_.value(), reassembler_.writer().bytes_pushed() ) - 1;
    reassembler_.insert( stream_index, message.payload, message.FIN );
  }
}

TCPReceiverMessage TCPReceiver::send() const
{
  TCPReceiverMessage ack_msg {};
  if ( isn_ ) {
    uint64_t abs_ackno = reassembler_.writer().bytes_pushed() + 1 + reassembler_.writer().is_closed();
    ack_msg.ackno = isn_.value() + abs_ackno;
  }
  ack_msg.window_size = min( reassembler_.writer().available_capacity(), static_cast<uint64_t>( UINT16_MAX ) );
  ack_msg.RST = reassembler_.writer().has_error();
  return ack_msg;
}
