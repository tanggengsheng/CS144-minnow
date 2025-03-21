#include "reassembler.hh"

using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring )
{
  if ( ( first_index + data.size() < first_unassembled_index_ )
       || ( first_index >= first_unassembled_index_ + output_.writer().available_capacity() ) ) {
    return;
  }

  if ( is_last_substring ) {
    end_of_index_ = first_index + data.size();
    eof_ = true;
  }

  if ( first_index < first_unassembled_index_ ) {
    data.erase( 0, first_unassembled_index_ - first_index );
    first_index = first_unassembled_index_;
  }

  if ( first_index + data.size() > first_unassembled_index_ + output_.writer().available_capacity() ) {
    data.erase( first_unassembled_index_ + output_.writer().available_capacity() - first_index );
  }

  if ( !unassembled_.empty() ) {
    auto upper = unassembled_.upper_bound( first_index );
    while ( ( upper != unassembled_.end() ) && ( upper->first <= first_index + data.size() ) ) {
      if ( first_index + data.size() < upper->first + upper->second.size() ) {
        data.append( upper->second, first_index + data.size() - upper->first );
      }
      bytes_pending_ -= upper->second.size();
      upper = unassembled_.erase( upper );
    }

    if ( !unassembled_.empty() && ( upper != unassembled_.begin() ) ) {
      auto lower = --upper;
      if ( lower->first + lower->second.size() >= first_index ) {
        if ( first_index + data.size() <= lower->first + lower->second.size() ) {
          return;
        }
        data.insert( 0, lower->second.substr( 0, first_index - lower->first ) );
        first_index = lower->first;
        bytes_pending_ -= lower->second.size();
        unassembled_.erase( lower );
      }
    }
  }

  if ( first_index == first_unassembled_index_ ) {
    output_.writer().push( data );
    first_unassembled_index_ += data.size();
  } else {
    unassembled_.insert( { first_index, data } );
    bytes_pending_ += data.size();
  }

  if ( eof_ && ( first_unassembled_index_ == end_of_index_ ) ) {
    output_.writer().close();
  }
}

// How many bytes are stored in the Reassembler itself?
// This function is for testing only; don't add extra state to support it.
uint64_t Reassembler::count_bytes_pending() const
{
  return bytes_pending_;
}
