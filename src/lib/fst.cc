// Copyright 2005-2020 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the 'License');
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an 'AS IS' BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// See www.openfst.org for extensive documentation on this weighted
// finite-state transducer library.
//
// FST definitions.

#include <fst/fst.h>

#include <sstream>

#include <fst/flags.h>
#include <fst/types.h>
#include <fst/log.h>
#include <fst/matcher-fst.h>  // declarations of *_lookahead_fst_type

// FST flag definitions.

DEFINE_int32(fst_v, 0, "FST log level, same as glog");

DEFINE_bool(fst_verify_properties, false,
            "Verify FST properties queried by TestProperties");

DEFINE_bool(fst_default_cache_gc, true, "Enable garbage collection of cache");

DEFINE_int64(fst_default_cache_gc_limit, 1 << 20LL,
             "Cache byte size that triggers garbage collection");

DEFINE_bool(fst_align, false, "Write FST data aligned where appropriate");

DEFINE_string(save_relabel_ipairs, "", "Save input relabel pairs to file");
DEFINE_string(save_relabel_opairs, "", "Save output relabel pairs to file");

DEFINE_string(fst_read_mode, "read",
              "Default file reading mode for mappable files");

namespace fst {

// FST type definitions for lookahead FSTs.
const char arc_lookahead_fst_type[] = "arc_lookahead";
const char ilabel_lookahead_fst_type[] = "ilabel_lookahead";
const char olabel_lookahead_fst_type[] = "olabel_lookahead";

// Checks FST magic number and reads in the header; if rewind = true,
// the stream is repositioned before call if possible.
bool FstHeader::Read(std::istream &strm, const std::string &source,
                     bool rewind) {
  int64 pos = 0;
  if (rewind) pos = strm.tellg();
  int32 magic_number = 0;
  ReadType(strm, &magic_number);
  if (magic_number != kFstMagicNumber) {
    LOG(ERROR) << "FstHeader::Read: Bad FST header: " << source
               << ". Magic number not matched. Got: " << magic_number;
    if (rewind) strm.seekg(pos);
    return false;
  }
  ReadType(strm, &fsttype_);
  ReadType(strm, &arctype_);
  ReadType(strm, &version_);
  ReadType(strm, &flags_);
  ReadType(strm, &properties_);
  ReadType(strm, &start_);
  ReadType(strm, &numstates_);
  ReadType(strm, &numarcs_);
  if (!strm) {
    LOG(ERROR) << "FstHeader::Read: Read failed: " << source;
    return false;
  }
  if (rewind) strm.seekg(pos);
  return true;
}

// Writes FST magic number and FST header.
bool FstHeader::Write(std::ostream &strm, const std::string &) const {
  WriteType(strm, kFstMagicNumber);
  WriteType(strm, fsttype_);
  WriteType(strm, arctype_);
  WriteType(strm, version_);
  WriteType(strm, flags_);
  WriteType(strm, properties_);
  WriteType(strm, start_);
  WriteType(strm, numstates_);
  WriteType(strm, numarcs_);
  return true;
}

std::string FstHeader::DebugString() const {
  std::ostringstream ostrm;
  ostrm << "fsttype: \"" << fsttype_ << "\" arctype: \"" << arctype_
        << "\" version: \"" << version_ << "\" flags: \"" << flags_
        << "\" properties: \"" << properties_ << "\" start: \"" << start_
        << "\" numstates: \"" << numstates_ << "\" numarcs: \"" << numarcs_
        << "\"";
  return ostrm.str();
}

FstReadOptions::FstReadOptions(const std::string_view source,
                               const FstHeader *header,
                               const SymbolTable *isymbols,
                               const SymbolTable *osymbols)
    : source(source),
      header(header),
      isymbols(isymbols),
      osymbols(osymbols),
      read_isymbols(true),
      read_osymbols(true) {
  mode = ReadMode(FLAGS_fst_read_mode);
}

FstReadOptions::FstReadOptions(const std::string_view source,
                               const SymbolTable *isymbols,
                               const SymbolTable *osymbols)
    : source(source),
      header(nullptr),
      isymbols(isymbols),
      osymbols(osymbols),
      read_isymbols(true),
      read_osymbols(true) {
  mode = ReadMode(FLAGS_fst_read_mode);
}

FstReadOptions::FileReadMode FstReadOptions::ReadMode(const std::string &mode) {
  if (mode == "read") return READ;
  if (mode == "map") return MAP;
  LOG(ERROR) << "Unknown file read mode " << mode;
  return READ;
}

std::string FstReadOptions::DebugString() const {
  std::ostringstream ostrm;
  ostrm << "source: \"" << source << "\" mode: \""
        << (mode == READ ? "READ" : "MAP") << "\" read_isymbols: \""
        << (read_isymbols ? "true" : "false") << "\" read_osymbols: \""
        << (read_osymbols ? "true" : "false") << "\" header: \""
        << (header ? "set" : "null") << "\" isymbols: \""
        << (isymbols ? "set" : "null") << "\" osymbols: \""
        << (osymbols ? "set" : "null") << "\"";
  return ostrm.str();
}

}  // namespace fst
