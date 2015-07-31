#include <iostream>
#include <cstdio>
#include "kseq.hpp"

kseq::kseq()
{
    this->last_char = 0;
}

kseq::~kseq()
{}

bool kseq::empty() {
  return name.empty() && comment.empty() && seq.empty() && qual.empty();
}
size_t kseq::size() {
  return seq.size();
}
size_t kseq::length() {
  return size();
}
bool kseq::isFasta() {
  return qual.empty();
}
void kseq::clear() {
  name.clear();
  comment.clear();
  seq.clear();
  qual.clear();
}

void kseq::print(FILE * out) {
  if(!empty()) {
    if(isFasta()) {
      fprintf(out, ">%s", name.c_str());
      if(!comment.empty()) {
        fprintf(out, "%s",comment.c_str());
      }
      fprintf(out, "\n%s\n", seq.c_str());
    } else {
      fprintf(out, "@%s", name.c_str());
      if(!comment.empty()) {
        fprintf(out, "%s", comment.c_str());
      }
      fprintf(out, "\n%s\n+\n%s\n", seq.c_str(), qual.c_str());
    }
  }
}

std::ostream& operator<<(std::ostream& out, kseq& mate) {
    if(mate.empty()) {
        return out;
    }
    if(mate.isFasta()) {
        out << ">"<<mate.name;
        if(!mate.comment.empty())
            out<<mate.comment;
        out<<"\n"<<mate.seq<<"\n";
    } else {
        out << "@"<<mate.name;
        if(!mate.comment.empty())
            out<<mate.comment;
        out<<"\n"<<mate.seq<<"\n+\n"<<mate.qual<<"\n";
    }
    return out;
}
