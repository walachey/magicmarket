// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: Expert.proto

#ifndef PROTOBUF_Expert_2eproto__INCLUDED
#define PROTOBUF_Expert_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 3000000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 3000000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/arena.h>
#include <google/protobuf/arenastring.h>
#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/metadata.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/generated_enum_reflection.h>
#include <google/protobuf/unknown_field_set.h>
// @@protoc_insertion_point(includes)

namespace Interfaces {

// Internal implementation detail -- do not call these.
void protobuf_AddDesc_Expert_2eproto();
void protobuf_AssignDesc_Expert_2eproto();
void protobuf_ShutdownFile_Expert_2eproto();

class ExpertMessage;
class ExpertMessage_Estimation;
class ExpertMessage_Information;

enum ExpertMessage_Type {
  ExpertMessage_Type_getName = 0,
  ExpertMessage_Type_getPrediction = 1,
  ExpertMessage_Type_getRequiredVariables = 2,
  ExpertMessage_Type_shutdown = 3,
  ExpertMessage_Type_reset = 4,
  ExpertMessage_Type_informations = 5
};
bool ExpertMessage_Type_IsValid(int value);
const ExpertMessage_Type ExpertMessage_Type_Type_MIN = ExpertMessage_Type_getName;
const ExpertMessage_Type ExpertMessage_Type_Type_MAX = ExpertMessage_Type_informations;
const int ExpertMessage_Type_Type_ARRAYSIZE = ExpertMessage_Type_Type_MAX + 1;

const ::google::protobuf::EnumDescriptor* ExpertMessage_Type_descriptor();
inline const ::std::string& ExpertMessage_Type_Name(ExpertMessage_Type value) {
  return ::google::protobuf::internal::NameOfEnum(
    ExpertMessage_Type_descriptor(), value);
}
inline bool ExpertMessage_Type_Parse(
    const ::std::string& name, ExpertMessage_Type* value) {
  return ::google::protobuf::internal::ParseNamedEnum<ExpertMessage_Type>(
    ExpertMessage_Type_descriptor(), name, value);
}
// ===================================================================

class ExpertMessage_Estimation : public ::google::protobuf::Message {
 public:
  ExpertMessage_Estimation();
  virtual ~ExpertMessage_Estimation();

  ExpertMessage_Estimation(const ExpertMessage_Estimation& from);

  inline ExpertMessage_Estimation& operator=(const ExpertMessage_Estimation& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ExpertMessage_Estimation& default_instance();

  void Swap(ExpertMessage_Estimation* other);

  // implements Message ----------------------------------------------

  inline ExpertMessage_Estimation* New() const { return New(NULL); }

  ExpertMessage_Estimation* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ExpertMessage_Estimation& from);
  void MergeFrom(const ExpertMessage_Estimation& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(ExpertMessage_Estimation* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required double mood = 1;
  bool has_mood() const;
  void clear_mood();
  static const int kMoodFieldNumber = 1;
  double mood() const;
  void set_mood(double value);

  // required double certainty = 2;
  bool has_certainty() const;
  void clear_certainty();
  static const int kCertaintyFieldNumber = 2;
  double certainty() const;
  void set_certainty(double value);

  // @@protoc_insertion_point(class_scope:Interfaces.ExpertMessage.Estimation)
 private:
  inline void set_has_mood();
  inline void clear_has_mood();
  inline void set_has_certainty();
  inline void clear_has_certainty();

  // helper for ByteSize()
  int RequiredFieldsByteSizeFallback() const;

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  double mood_;
  double certainty_;
  friend void  protobuf_AddDesc_Expert_2eproto();
  friend void protobuf_AssignDesc_Expert_2eproto();
  friend void protobuf_ShutdownFile_Expert_2eproto();

  void InitAsDefaultInstance();
  static ExpertMessage_Estimation* default_instance_;
};
// -------------------------------------------------------------------

class ExpertMessage_Information : public ::google::protobuf::Message {
 public:
  ExpertMessage_Information();
  virtual ~ExpertMessage_Information();

  ExpertMessage_Information(const ExpertMessage_Information& from);

  inline ExpertMessage_Information& operator=(const ExpertMessage_Information& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ExpertMessage_Information& default_instance();

  void Swap(ExpertMessage_Information* other);

  // implements Message ----------------------------------------------

  inline ExpertMessage_Information* New() const { return New(NULL); }

  ExpertMessage_Information* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ExpertMessage_Information& from);
  void MergeFrom(const ExpertMessage_Information& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(ExpertMessage_Information* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // required bool isExecutive = 1;
  bool has_isexecutive() const;
  void clear_isexecutive();
  static const int kIsExecutiveFieldNumber = 1;
  bool isexecutive() const;
  void set_isexecutive(bool value);

  // @@protoc_insertion_point(class_scope:Interfaces.ExpertMessage.Information)
 private:
  inline void set_has_isexecutive();
  inline void clear_has_isexecutive();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  bool isexecutive_;
  friend void  protobuf_AddDesc_Expert_2eproto();
  friend void protobuf_AssignDesc_Expert_2eproto();
  friend void protobuf_ShutdownFile_Expert_2eproto();

  void InitAsDefaultInstance();
  static ExpertMessage_Information* default_instance_;
};
// -------------------------------------------------------------------

class ExpertMessage : public ::google::protobuf::Message {
 public:
  ExpertMessage();
  virtual ~ExpertMessage();

  ExpertMessage(const ExpertMessage& from);

  inline ExpertMessage& operator=(const ExpertMessage& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _internal_metadata_.unknown_fields();
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return _internal_metadata_.mutable_unknown_fields();
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const ExpertMessage& default_instance();

  void Swap(ExpertMessage* other);

  // implements Message ----------------------------------------------

  inline ExpertMessage* New() const { return New(NULL); }

  ExpertMessage* New(::google::protobuf::Arena* arena) const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const ExpertMessage& from);
  void MergeFrom(const ExpertMessage& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  void InternalSwap(ExpertMessage* other);
  private:
  inline ::google::protobuf::Arena* GetArenaNoVirtual() const {
    return _internal_metadata_.arena();
  }
  inline void* MaybeArenaPtr() const {
    return _internal_metadata_.raw_arena_ptr();
  }
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  typedef ExpertMessage_Estimation Estimation;
  typedef ExpertMessage_Information Information;

  typedef ExpertMessage_Type Type;
  static const Type getName = ExpertMessage_Type_getName;
  static const Type getPrediction = ExpertMessage_Type_getPrediction;
  static const Type getRequiredVariables = ExpertMessage_Type_getRequiredVariables;
  static const Type shutdown = ExpertMessage_Type_shutdown;
  static const Type reset = ExpertMessage_Type_reset;
  static const Type informations = ExpertMessage_Type_informations;
  static inline bool Type_IsValid(int value) {
    return ExpertMessage_Type_IsValid(value);
  }
  static const Type Type_MIN =
    ExpertMessage_Type_Type_MIN;
  static const Type Type_MAX =
    ExpertMessage_Type_Type_MAX;
  static const int Type_ARRAYSIZE =
    ExpertMessage_Type_Type_ARRAYSIZE;
  static inline const ::google::protobuf::EnumDescriptor*
  Type_descriptor() {
    return ExpertMessage_Type_descriptor();
  }
  static inline const ::std::string& Type_Name(Type value) {
    return ExpertMessage_Type_Name(value);
  }
  static inline bool Type_Parse(const ::std::string& name,
      Type* value) {
    return ExpertMessage_Type_Parse(name, value);
  }

  // accessors -------------------------------------------------------

  // required .Interfaces.ExpertMessage.Type type = 1;
  bool has_type() const;
  void clear_type();
  static const int kTypeFieldNumber = 1;
  ::Interfaces::ExpertMessage_Type type() const;
  void set_type(::Interfaces::ExpertMessage_Type value);

  // optional string name = 2;
  bool has_name() const;
  void clear_name();
  static const int kNameFieldNumber = 2;
  const ::std::string& name() const;
  void set_name(const ::std::string& value);
  void set_name(const char* value);
  void set_name(const char* value, size_t size);
  ::std::string* mutable_name();
  ::std::string* release_name();
  void set_allocated_name(::std::string* name);

  // repeated string variableNames = 3;
  int variablenames_size() const;
  void clear_variablenames();
  static const int kVariableNamesFieldNumber = 3;
  const ::std::string& variablenames(int index) const;
  ::std::string* mutable_variablenames(int index);
  void set_variablenames(int index, const ::std::string& value);
  void set_variablenames(int index, const char* value);
  void set_variablenames(int index, const char* value, size_t size);
  ::std::string* add_variablenames();
  void add_variablenames(const ::std::string& value);
  void add_variablenames(const char* value);
  void add_variablenames(const char* value, size_t size);
  const ::google::protobuf::RepeatedPtrField< ::std::string>& variablenames() const;
  ::google::protobuf::RepeatedPtrField< ::std::string>* mutable_variablenames();

  // repeated double variables = 4;
  int variables_size() const;
  void clear_variables();
  static const int kVariablesFieldNumber = 4;
  double variables(int index) const;
  void set_variables(int index, double value);
  void add_variables(double value);
  const ::google::protobuf::RepeatedField< double >&
      variables() const;
  ::google::protobuf::RepeatedField< double >*
      mutable_variables();

  // optional .Interfaces.ExpertMessage.Estimation estimation = 5;
  bool has_estimation() const;
  void clear_estimation();
  static const int kEstimationFieldNumber = 5;
  const ::Interfaces::ExpertMessage_Estimation& estimation() const;
  ::Interfaces::ExpertMessage_Estimation* mutable_estimation();
  ::Interfaces::ExpertMessage_Estimation* release_estimation();
  void set_allocated_estimation(::Interfaces::ExpertMessage_Estimation* estimation);

  // optional .Interfaces.ExpertMessage.Information information = 6;
  bool has_information() const;
  void clear_information();
  static const int kInformationFieldNumber = 6;
  const ::Interfaces::ExpertMessage_Information& information() const;
  ::Interfaces::ExpertMessage_Information* mutable_information();
  ::Interfaces::ExpertMessage_Information* release_information();
  void set_allocated_information(::Interfaces::ExpertMessage_Information* information);

  // @@protoc_insertion_point(class_scope:Interfaces.ExpertMessage)
 private:
  inline void set_has_type();
  inline void clear_has_type();
  inline void set_has_name();
  inline void clear_has_name();
  inline void set_has_estimation();
  inline void clear_has_estimation();
  inline void set_has_information();
  inline void clear_has_information();

  ::google::protobuf::internal::InternalMetadataWithArena _internal_metadata_;
  ::google::protobuf::uint32 _has_bits_[1];
  mutable int _cached_size_;
  ::google::protobuf::internal::ArenaStringPtr name_;
  ::google::protobuf::RepeatedPtrField< ::std::string> variablenames_;
  ::google::protobuf::RepeatedField< double > variables_;
  ::Interfaces::ExpertMessage_Estimation* estimation_;
  ::Interfaces::ExpertMessage_Information* information_;
  int type_;
  friend void  protobuf_AddDesc_Expert_2eproto();
  friend void protobuf_AssignDesc_Expert_2eproto();
  friend void protobuf_ShutdownFile_Expert_2eproto();

  void InitAsDefaultInstance();
  static ExpertMessage* default_instance_;
};
// ===================================================================


// ===================================================================

#if !PROTOBUF_INLINE_NOT_IN_HEADERS
// ExpertMessage_Estimation

// required double mood = 1;
inline bool ExpertMessage_Estimation::has_mood() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void ExpertMessage_Estimation::set_has_mood() {
  _has_bits_[0] |= 0x00000001u;
}
inline void ExpertMessage_Estimation::clear_has_mood() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void ExpertMessage_Estimation::clear_mood() {
  mood_ = 0;
  clear_has_mood();
}
inline double ExpertMessage_Estimation::mood() const {
  // @@protoc_insertion_point(field_get:Interfaces.ExpertMessage.Estimation.mood)
  return mood_;
}
inline void ExpertMessage_Estimation::set_mood(double value) {
  set_has_mood();
  mood_ = value;
  // @@protoc_insertion_point(field_set:Interfaces.ExpertMessage.Estimation.mood)
}

// required double certainty = 2;
inline bool ExpertMessage_Estimation::has_certainty() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void ExpertMessage_Estimation::set_has_certainty() {
  _has_bits_[0] |= 0x00000002u;
}
inline void ExpertMessage_Estimation::clear_has_certainty() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void ExpertMessage_Estimation::clear_certainty() {
  certainty_ = 0;
  clear_has_certainty();
}
inline double ExpertMessage_Estimation::certainty() const {
  // @@protoc_insertion_point(field_get:Interfaces.ExpertMessage.Estimation.certainty)
  return certainty_;
}
inline void ExpertMessage_Estimation::set_certainty(double value) {
  set_has_certainty();
  certainty_ = value;
  // @@protoc_insertion_point(field_set:Interfaces.ExpertMessage.Estimation.certainty)
}

// -------------------------------------------------------------------

// ExpertMessage_Information

// required bool isExecutive = 1;
inline bool ExpertMessage_Information::has_isexecutive() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void ExpertMessage_Information::set_has_isexecutive() {
  _has_bits_[0] |= 0x00000001u;
}
inline void ExpertMessage_Information::clear_has_isexecutive() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void ExpertMessage_Information::clear_isexecutive() {
  isexecutive_ = false;
  clear_has_isexecutive();
}
inline bool ExpertMessage_Information::isexecutive() const {
  // @@protoc_insertion_point(field_get:Interfaces.ExpertMessage.Information.isExecutive)
  return isexecutive_;
}
inline void ExpertMessage_Information::set_isexecutive(bool value) {
  set_has_isexecutive();
  isexecutive_ = value;
  // @@protoc_insertion_point(field_set:Interfaces.ExpertMessage.Information.isExecutive)
}

// -------------------------------------------------------------------

// ExpertMessage

// required .Interfaces.ExpertMessage.Type type = 1;
inline bool ExpertMessage::has_type() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void ExpertMessage::set_has_type() {
  _has_bits_[0] |= 0x00000001u;
}
inline void ExpertMessage::clear_has_type() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void ExpertMessage::clear_type() {
  type_ = 0;
  clear_has_type();
}
inline ::Interfaces::ExpertMessage_Type ExpertMessage::type() const {
  // @@protoc_insertion_point(field_get:Interfaces.ExpertMessage.type)
  return static_cast< ::Interfaces::ExpertMessage_Type >(type_);
}
inline void ExpertMessage::set_type(::Interfaces::ExpertMessage_Type value) {
  assert(::Interfaces::ExpertMessage_Type_IsValid(value));
  set_has_type();
  type_ = value;
  // @@protoc_insertion_point(field_set:Interfaces.ExpertMessage.type)
}

// optional string name = 2;
inline bool ExpertMessage::has_name() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void ExpertMessage::set_has_name() {
  _has_bits_[0] |= 0x00000002u;
}
inline void ExpertMessage::clear_has_name() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void ExpertMessage::clear_name() {
  name_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  clear_has_name();
}
inline const ::std::string& ExpertMessage::name() const {
  // @@protoc_insertion_point(field_get:Interfaces.ExpertMessage.name)
  return name_.GetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ExpertMessage::set_name(const ::std::string& value) {
  set_has_name();
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), value);
  // @@protoc_insertion_point(field_set:Interfaces.ExpertMessage.name)
}
inline void ExpertMessage::set_name(const char* value) {
  set_has_name();
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), ::std::string(value));
  // @@protoc_insertion_point(field_set_char:Interfaces.ExpertMessage.name)
}
inline void ExpertMessage::set_name(const char* value, size_t size) {
  set_has_name();
  name_.SetNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(),
      ::std::string(reinterpret_cast<const char*>(value), size));
  // @@protoc_insertion_point(field_set_pointer:Interfaces.ExpertMessage.name)
}
inline ::std::string* ExpertMessage::mutable_name() {
  set_has_name();
  // @@protoc_insertion_point(field_mutable:Interfaces.ExpertMessage.name)
  return name_.MutableNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline ::std::string* ExpertMessage::release_name() {
  clear_has_name();
  return name_.ReleaseNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
}
inline void ExpertMessage::set_allocated_name(::std::string* name) {
  if (name != NULL) {
    set_has_name();
  } else {
    clear_has_name();
  }
  name_.SetAllocatedNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), name);
  // @@protoc_insertion_point(field_set_allocated:Interfaces.ExpertMessage.name)
}

// repeated string variableNames = 3;
inline int ExpertMessage::variablenames_size() const {
  return variablenames_.size();
}
inline void ExpertMessage::clear_variablenames() {
  variablenames_.Clear();
}
inline const ::std::string& ExpertMessage::variablenames(int index) const {
  // @@protoc_insertion_point(field_get:Interfaces.ExpertMessage.variableNames)
  return variablenames_.Get(index);
}
inline ::std::string* ExpertMessage::mutable_variablenames(int index) {
  // @@protoc_insertion_point(field_mutable:Interfaces.ExpertMessage.variableNames)
  return variablenames_.Mutable(index);
}
inline void ExpertMessage::set_variablenames(int index, const ::std::string& value) {
  // @@protoc_insertion_point(field_set:Interfaces.ExpertMessage.variableNames)
  variablenames_.Mutable(index)->assign(value);
}
inline void ExpertMessage::set_variablenames(int index, const char* value) {
  variablenames_.Mutable(index)->assign(value);
  // @@protoc_insertion_point(field_set_char:Interfaces.ExpertMessage.variableNames)
}
inline void ExpertMessage::set_variablenames(int index, const char* value, size_t size) {
  variablenames_.Mutable(index)->assign(
    reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_set_pointer:Interfaces.ExpertMessage.variableNames)
}
inline ::std::string* ExpertMessage::add_variablenames() {
  return variablenames_.Add();
}
inline void ExpertMessage::add_variablenames(const ::std::string& value) {
  variablenames_.Add()->assign(value);
  // @@protoc_insertion_point(field_add:Interfaces.ExpertMessage.variableNames)
}
inline void ExpertMessage::add_variablenames(const char* value) {
  variablenames_.Add()->assign(value);
  // @@protoc_insertion_point(field_add_char:Interfaces.ExpertMessage.variableNames)
}
inline void ExpertMessage::add_variablenames(const char* value, size_t size) {
  variablenames_.Add()->assign(reinterpret_cast<const char*>(value), size);
  // @@protoc_insertion_point(field_add_pointer:Interfaces.ExpertMessage.variableNames)
}
inline const ::google::protobuf::RepeatedPtrField< ::std::string>&
ExpertMessage::variablenames() const {
  // @@protoc_insertion_point(field_list:Interfaces.ExpertMessage.variableNames)
  return variablenames_;
}
inline ::google::protobuf::RepeatedPtrField< ::std::string>*
ExpertMessage::mutable_variablenames() {
  // @@protoc_insertion_point(field_mutable_list:Interfaces.ExpertMessage.variableNames)
  return &variablenames_;
}

// repeated double variables = 4;
inline int ExpertMessage::variables_size() const {
  return variables_.size();
}
inline void ExpertMessage::clear_variables() {
  variables_.Clear();
}
inline double ExpertMessage::variables(int index) const {
  // @@protoc_insertion_point(field_get:Interfaces.ExpertMessage.variables)
  return variables_.Get(index);
}
inline void ExpertMessage::set_variables(int index, double value) {
  variables_.Set(index, value);
  // @@protoc_insertion_point(field_set:Interfaces.ExpertMessage.variables)
}
inline void ExpertMessage::add_variables(double value) {
  variables_.Add(value);
  // @@protoc_insertion_point(field_add:Interfaces.ExpertMessage.variables)
}
inline const ::google::protobuf::RepeatedField< double >&
ExpertMessage::variables() const {
  // @@protoc_insertion_point(field_list:Interfaces.ExpertMessage.variables)
  return variables_;
}
inline ::google::protobuf::RepeatedField< double >*
ExpertMessage::mutable_variables() {
  // @@protoc_insertion_point(field_mutable_list:Interfaces.ExpertMessage.variables)
  return &variables_;
}

// optional .Interfaces.ExpertMessage.Estimation estimation = 5;
inline bool ExpertMessage::has_estimation() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void ExpertMessage::set_has_estimation() {
  _has_bits_[0] |= 0x00000010u;
}
inline void ExpertMessage::clear_has_estimation() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void ExpertMessage::clear_estimation() {
  if (estimation_ != NULL) estimation_->::Interfaces::ExpertMessage_Estimation::Clear();
  clear_has_estimation();
}
inline const ::Interfaces::ExpertMessage_Estimation& ExpertMessage::estimation() const {
  // @@protoc_insertion_point(field_get:Interfaces.ExpertMessage.estimation)
  return estimation_ != NULL ? *estimation_ : *default_instance_->estimation_;
}
inline ::Interfaces::ExpertMessage_Estimation* ExpertMessage::mutable_estimation() {
  set_has_estimation();
  if (estimation_ == NULL) {
    estimation_ = new ::Interfaces::ExpertMessage_Estimation;
  }
  // @@protoc_insertion_point(field_mutable:Interfaces.ExpertMessage.estimation)
  return estimation_;
}
inline ::Interfaces::ExpertMessage_Estimation* ExpertMessage::release_estimation() {
  clear_has_estimation();
  ::Interfaces::ExpertMessage_Estimation* temp = estimation_;
  estimation_ = NULL;
  return temp;
}
inline void ExpertMessage::set_allocated_estimation(::Interfaces::ExpertMessage_Estimation* estimation) {
  delete estimation_;
  estimation_ = estimation;
  if (estimation) {
    set_has_estimation();
  } else {
    clear_has_estimation();
  }
  // @@protoc_insertion_point(field_set_allocated:Interfaces.ExpertMessage.estimation)
}

// optional .Interfaces.ExpertMessage.Information information = 6;
inline bool ExpertMessage::has_information() const {
  return (_has_bits_[0] & 0x00000020u) != 0;
}
inline void ExpertMessage::set_has_information() {
  _has_bits_[0] |= 0x00000020u;
}
inline void ExpertMessage::clear_has_information() {
  _has_bits_[0] &= ~0x00000020u;
}
inline void ExpertMessage::clear_information() {
  if (information_ != NULL) information_->::Interfaces::ExpertMessage_Information::Clear();
  clear_has_information();
}
inline const ::Interfaces::ExpertMessage_Information& ExpertMessage::information() const {
  // @@protoc_insertion_point(field_get:Interfaces.ExpertMessage.information)
  return information_ != NULL ? *information_ : *default_instance_->information_;
}
inline ::Interfaces::ExpertMessage_Information* ExpertMessage::mutable_information() {
  set_has_information();
  if (information_ == NULL) {
    information_ = new ::Interfaces::ExpertMessage_Information;
  }
  // @@protoc_insertion_point(field_mutable:Interfaces.ExpertMessage.information)
  return information_;
}
inline ::Interfaces::ExpertMessage_Information* ExpertMessage::release_information() {
  clear_has_information();
  ::Interfaces::ExpertMessage_Information* temp = information_;
  information_ = NULL;
  return temp;
}
inline void ExpertMessage::set_allocated_information(::Interfaces::ExpertMessage_Information* information) {
  delete information_;
  information_ = information;
  if (information) {
    set_has_information();
  } else {
    clear_has_information();
  }
  // @@protoc_insertion_point(field_set_allocated:Interfaces.ExpertMessage.information)
}

#endif  // !PROTOBUF_INLINE_NOT_IN_HEADERS
// -------------------------------------------------------------------

// -------------------------------------------------------------------


// @@protoc_insertion_point(namespace_scope)

}  // namespace Interfaces

#ifndef SWIG
namespace google {
namespace protobuf {

template <> struct is_proto_enum< ::Interfaces::ExpertMessage_Type> : ::google::protobuf::internal::true_type {};
template <>
inline const EnumDescriptor* GetEnumDescriptor< ::Interfaces::ExpertMessage_Type>() {
  return ::Interfaces::ExpertMessage_Type_descriptor();
}

}  // namespace protobuf
}  // namespace google
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_Expert_2eproto__INCLUDED