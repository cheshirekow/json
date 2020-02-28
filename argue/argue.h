#pragma once
// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include <array>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "util/container_of.h"
#include "util/type_string.h"

#define ARGUE_VERSION \
  { 0, 1, 2 }

// An command line argument parsing library
namespace argue {

// =============================================================================
//                                 Utilities
// =============================================================================

// Convert a string to all upper-case
std::string ToUpper(const std::string& str);

// Convert a string to all lower-case
std::string ToLower(const std::string& str);

// Return true if the query is a valid choice (i.e. matches with equality an
// element of the vector)
template <typename T>
bool HasChoice(const std::vector<T>& choices, const T& query);

// Create a string by joining the elements of the container using default
// stream formatting and the provided delimeter. Uses
// ``ostream& operator<<(ostream&, ...)`` to format each element
template <typename Container>
std::string Join(const Container& container, const std::string& delim = ", ");

// Return a vector of the keys of an associative container (i.e. std::map)
template <typename Container>
std::vector<typename Container::key_type> Keys(const Container& container);

// Compute the sum of all the elements of a container
template <typename Container>
typename Container::value_type ContainerSum(const Container& container);

// Template metaprogram evaluates to the value type of a container or the
// input type of a non container. The default template is for scalar types
// and just evaluates to the input type.
template <typename Scalar>
class ElementType {
 public:
  typedef Scalar value;
};

// Specialization for std::list, evaluates to the value type
template <typename T, class Allocator>
class ElementType<std::list<T, Allocator>> {
 public:
  typedef T value;
};

// Specialization for std::vector, evaluates to the value type
template <typename T, class Allocator>
class ElementType<std::vector<T, Allocator>> {
 public:
  typedef T value;
};

// =============================================================================
//                    Exception Handling and Stack Traces
// =============================================================================

// Element type for a stack trace
// see: http://man7.org/linux/man-pages/man3/backtrace.3.html
struct TraceLine {
  void* addr;          //< symbol address
  std::string file;    //< file from which the symbol originated
  std::string name;    //< symbol name
  std::string offset;  //< string representation of the stack pointer offset
                       // from the symbol
  std::string saddr;   //< string representation of the symbol address
};

// A stack trace is just a vector of stack line information
typedef std::vector<TraceLine> StackTrace;

// Return the current stack trace. ``skip_frames`` defaults to 2 because
// the argue::Assertion adds two calls to the stack.
StackTrace GetStacktrace(size_t skip_frames = 1, size_t max_frames = 50);

// Print the stack trace line by line to the output stream.
std::ostream& operator<<(std::ostream& out, const StackTrace& trace);

// Exceptions thrown by this library are all of this type.
class Exception : public std::exception {
 public:
  // Signify severity of the exception, and whether it's an exception due
  // to argue itself, the library user who is writing a parser, or the
  // program user who is calling the program with arguments.
  enum TypeNo {
    BUG = 0,       // A bug in the library code
    CONFIG_ERROR,  // Library user error
    INPUT_ERROR,   // Program user error
  };

  Exception(TypeNo typeno, const std::string& message)
      : typeno(typeno), message(message) {}
  const char* what() const noexcept override;
  static const char* ToString(TypeNo typeno);

  TypeNo typeno;
  std::string file;
  int lineno;
  std::string message;
  StackTrace stack_trace;
};

// Stores the file name and line number of an assertion exception.
/* An assetion sentinel captures the current file and line number so that the
 * correct value is used, even if the message occurs at a later line. It also
 * serves as an indicator to the compiler which overload of operator&&() to
 * use during assertions. */
struct AssertionSentinel {
  std::string file;
  int lineno;
};

// Exception message builder
/* Provides state and stream operators so that complex exception messages can
 * be conveniently composed. The purpose of an Assertion object is just to
 * hold the exception type and message used to construct an exception. The
 * exception itself is not constructructed and thrown until the
 * `operator&&` is called */
class Assertion {
 public:
  // message will be appended
  explicit Assertion(Exception::TypeNo typeno, bool expr);

  // construct with message
  Assertion(Exception::TypeNo typeno, bool expr, const std::string& message);

  template <typename T>
  Assertion& operator<<(const T& x);

  template <typename T>
  Assertion& operator<<(const T&& x);

  Exception::TypeNo typeno;   //< The exception type number
  bool expr_;                 //< The boolean expression being evaluated
  std::stringstream sstream;  //< stringstream to store the intermediate message
                              // as it's being constructed
};

// Construct and throw an exception
/* The && operator has lower precidence than the << operator so this allows
 * us to raise the exception after the message has been constructed but before
 * the assertion object has been destroyed.
 *
 * Usage:
 *
 * ```
 * AssertionSentinel(...) && Assertion(...) << "Message";
 * ```
 */
void operator&&(const argue::AssertionSentinel& sentinel,
                const argue::Assertion& assertion);

// Evaluate an assertion (boolean expression) and if false throw an exception
/* Use this macro is as if it was a function with the following signature:
 *
 * ```
 * ostream& ARGUE_ASSERT(Exception::TypeNo class, bool expr);
 * ```
 * e.g.
 * ```
 * ARGUE_ASSERT(INPUT_ERROR, a == b)
 *  << "Message line 1\n"
 *  << "Message line 2\n";
 * ``` */
#define ARGUE_ASSERT(typeno, ...)                     \
  ::argue::AssertionSentinel({__FILE__, __LINE__}) && \
      ::argue::Assertion(::argue::Exception::typeno, __VA_ARGS__)

// Throw an exception using the exception message builder
/* Use this macro as if it was a function with following signatures:
 *
 * ```
 * ostream& ARGUE_THROW(Exception::TypeNo class);
 * ```
 *
 * e.g.
 *
 * ```
 * ARGUE_THROW(INPUT_ERROR)
 *  << "Message line 1\n"
 *  << "Message line 2\n";
 * ``` */
#define ARGUE_THROW(typeno)                           \
  ::argue::AssertionSentinel({__FILE__, __LINE__}) && \
      ::argue::Assertion(::argue::Exception::typeno, false)

// =============================================================================
//                          String Parsing
// =============================================================================

// Parse a base-10 string as into a signed integer. Matches strings of the
// form `[-?]\d+`.
template <typename T>
int ParseSigned(const std::string& str, T* value);

// Parse a base-10 string into an unsigned integer. Matches strings of the
// form `\d+`.
template <typename T>
int ParseUnsigned(const std::string& str, T* value);

// Parse a real-number string into a floating point value. Matches strings
// of the form `[-?]\d+\.?\d*`
template <typename T>
int ParseFloat(const std::string& str, T* value);

int Parse(const std::string& str, uint8_t* value);
int Parse(const std::string& str, uint16_t* value);
int Parse(const std::string& str, uint32_t* value);
int Parse(const std::string& str, uint64_t* value);
int Parse(const std::string& str, int8_t* value);
int Parse(const std::string& str, int16_t* value);
int Parse(const std::string& str, int32_t* value);
int Parse(const std::string& str, int64_t* value);
int Parse(const std::string& str, float* value);
int Parse(const std::string& str, double* value);
int Parse(const std::string& str, bool* value);
int Parse(const std::string& str, std::string* value);

template <typename T>
int Parse(const std::string& str, std::shared_ptr<T>* ptr);

template <typename T, class Allocator>
int Parse(const std::string& str, std::list<T, Allocator>* ptr);

template <typename T, class Allocator>
int Parse(const std::string& str, std::vector<T, Allocator>* ptr);

// Tokens in an argument list are one of these.
enum ArgType { SHORT_FLAG = 0, LONG_FLAG = 1, POSITIONAL = 2 };

// Return the ArgType of a string token:
//  * SHORT_FLAG if it is of the form `-[^-]`
//  * LONG_FLAG if it is of the form `--.+`
//  * POSITIONAL otherwise
ArgType GetArgType(const std::string arg);

// Sentinel integer values used to indicate special `nargs`.
enum SentinelNargs {
  REMAINDER = -7,      //< consume all the remaining arguments, regardless
                       // of whether or not they appear to be positional or
                       // flag, into a list of strings
  ZERO_NARGS = -6,     //< no arguments, only has meaning for `store_const`
                       //  style flags and things
  INVALID_NARGS = -5,  //< initial value, or sentinel output from
                       // parsing `nargs` strings
  ONE_OR_MORE = -4,    //< consume values until we reach a flag or run out of
                       //  arguments, but throw an error if we do not consume
                       //  at least one
  ZERO_OR_MORE = -3,   //< consume values until we reach a flag or run out of
                       //  arguments
  ZERO_OR_ONE = -2,    //< consume one value if available, so long as it is not
                       //  a flag
  EXACTLY_ONE = -1,    //< consume one value if available, so long as it is not
                       //  a flag. Throw an error if there are no arguments
                       //  remaining or the next argument is a flag
};

// Parse a string (i.e. '?', '*', '+') into a sentinel `nargs` value.
int StringToNargs(char key);

// Parse a string (i.e. '?', '*', '+') into a sentinel `nargs` value.
int StringToNargs(const char* str);

// Parse a string (i.e. '?', '*', '+') into a sentinel `nargs` value.
int StringToNargs(const std::string& str);

// =============================================================================
//                           Storage Model
// =============================================================================

// Abstract interface into different container types
/* All containers of a particular value type are wrapped by some class which
 * derives from StorageModel<T>, allowing us to interface with those containers
 * without knowing their remaining template parameters */
template <typename T>
class StorageModel {
 public:
  virtual ~StorageModel() {}

  // Initialize storage for a given lenth.
  /* Note that the capacity_hint might be zero, in the case of "append"
   * type actions, or narg="+""... in which case the storage should be
   * large enough to hold all the expected values, or should be growable. */
  virtual void Init(size_t capacity_hint) = 0;

  // Append an element to the list model
  virtual void Append(const T& value) = 0;

 protected:
  std::string type_name_;
};

// Abstract the interface into a `std::list`
template <typename T, class Allocator>
class ListModel : public StorageModel<T> {
 public:
  explicit ListModel(std::list<T, Allocator>* dest);
  virtual ~ListModel() {}

  void Init(size_t /*capacity_hint*/) override;
  void Append(const T& value) override;

 private:
  std::list<T, Allocator>* dest_;
};

// Abstract the interface into a `std::vector`
template <typename T, class Allocator>
class VectorModel : public StorageModel<T> {
 public:
  explicit VectorModel(std::vector<T, Allocator>* dest);
  virtual ~VectorModel() {}

  void Init(size_t capacity_hint);
  void Append(const T& value);

 private:
  std::vector<T, Allocator>* dest_;
};

// =============================================================================
//                              Actions
// =============================================================================

class Parser;

// Context provided to Action objects during argument parsing
struct ParseContext {
  Parser* parser;     //< Pointer to the parser that owns the argument
  std::ostream* out;  //< output stream where to write any messages
  std::string arg;    //< the argument that initiated this action, in the case
                      //  of actions associated with flags. Empty for
                      //  actions associated with positionals
};

// Enumerates the possible result cases from a call to `ParseArgs`.
enum ParseResult {
  PARSE_FINISHED = 0,   // Finished parsing arguments, no errors
  PARSE_ABORTED = 1,    // Parsing was terminated early but not in error,
                        // usually this means --help or --version
  PARSE_EXCEPTION = 2,  // An exception occured and parsing failed
  // TODO(josh): add separate cases for USER exception and DEVELOPER exception,
  // so that we know what to print on the console.
};

// Context filled by an action at it's conclusion
struct ActionResult {
  bool keep_active;  //< Set true by the action if it wishes to remain active
                     //  after processing. This is only meaningful for flags
                     //  and will be ignored for positionals.
  ParseResult code;  //< success/failure of the parse
};

// Indicates what kind of argument a particular action is associated with
enum Usage {
  USAGE_POSITIONAL = 0,  //< action is associated with a positional
  USAGE_FLAG             //< action is associated with a flag
};

// Interface shared by all action objects.
class ActionBase {
 public:
  ActionBase();
  virtual ~ActionBase();

  // Set the number of arguments accepted by the action.
  virtual void SetNargs(int nargs);

  // Set whether or not the argument is required. Note that assignment of this
  // value is only meaningful for actions associated with flags.
  virtual void SetRequired(bool required);

  // Set the help text used to describe the action
  virtual void SetHelp(const std::string& help);

  // Set the metavariable string used to placehold the argument values when
  // constructing help text
  virtual void SetMetavar(const std::string& metavar);

  // Set the usage (flag or positional) that the action is associated with
  virtual void SetUsage(Usage usage);

  // Return true if the action is fully and correctly configured.
  /* This is the best place to implement assertions regarding the configuration
   * of the action as they'll be caugh regardless of what command line arguments
   * are pumped through the parser.
   *
   * `Valdate()` has the side effect of assigning default values wherever
   * they have been configured.
   *
   * TODO(josh): move default assignment to a separate function! */
  virtual bool Validate();

  // Return true if the argument is required.
  /* This is used after all arguments are consumed to determine if the command
   * line was valid. If any arguments remain in the queue that are marked
   * required, then the parse fails.
   *
   * In general, an argument is required if:
   *  * it is a positional with nargs other than '+' or '*'
   *  * it is a flag and `{.required=true}` was specified
   * */
  virtual bool IsRequired() const;

  // Return the number of arguments consumed by this action.
  // TODO(josh): remove ``default_value``, just set the default value in the
  // constructor!
  virtual int GetNargs(int default_value) const;

  // Return the string used to represent the value of this argument in help
  // text.
  // TODO(josh): remove ``default_value``, just set the default in the
  // constructor!
  virtual std::string GetMetavar(const std::string& default_value) const;

  // Return right hand side of help text for the help table
  /* Generally this is composed of whatever was provided by `{.help=}` during
   * a call to `AddArgument`. However it will also include some generated
   * content like `choices` or `default` */
  virtual std::string GetHelp() const;

  // Parse zero or more argument values out of the list `args`
  /* Actions should modify args and leave it in a state consistent with
   * "remaining arguments". */
  virtual void ConsumeArgs(const ParseContext& ctx,
                           std::list<std::string>* args,
                           ActionResult* result) = 0;

 protected:
  std::string type_name_;

  uint32_t usage_ : 1;            //< USAGE_FLAGS or USAGE_POSITIONAL
  uint32_t has_nargs_ : 1;        //< true if nargs_ has been assigned
  uint32_t has_const_ : 1;        //< true if const_ has been assigned
  uint32_t has_default_ : 1;      //< true if default_ has been assigned
  uint32_t has_choices_ : 1;      //< true if choices_ has been assigned
  uint32_t has_required_ : 1;     //< true if required_ has been assigned
  uint32_t has_help_ : 1;         //< true if help_ has been assigned
  uint32_t has_metavar_ : 1;      //< true if metavar_ has been assigned
  uint32_t has_destination_ : 1;  //< true if destination_ has been assigned

  int nargs_;              //< number of arguments consumed by this action
  bool required_ = false;  //< true if this action is required, and if it should
                           //  be considered an error if this action remains
                           //  after all arguments are consumed
  std::string help_;       //< help text for this action
  std::string metavar_;    //< string to use in place of this actions values
                           //  when constructing usage or help text
};

// Interface shared by all action objects that support the standard setters.
/* TODO(josh): change name of ActionBase to Action and change name of
 * Action to StandardAction */
template <typename T>
class Action : public ActionBase {
 public:
  Action();
  virtual ~Action() {}

  virtual void SetConst(const T& value);
  virtual void SetDefault(const T& value);
  virtual void SetDefault(const std::vector<T>&& value);
  virtual void SetChoices(const std::vector<T>&& value);
  virtual void SetDestination(T* destination);
  virtual void SetDestination(
      const std::shared_ptr<StorageModel<T>>& destination);

 protected:
  // A list of the valid values allowed to be consumed by this action
  std::vector<T> choices_;
};

// Specialization for actions that don't support standard setters for data
/* Actions of this type still support the non-template setters like `SetHelp()`
 * or `SetMetavar` */
template <>
class Action<void> : public ActionBase {
 public:
  Action() {}
  virtual ~Action() {}
};

// Implements the "store" action for scalars
/* The "store" simply parses the string into a value of the correct
 * type and stores it in some variable. This does not includes list-y
 * variables which are handled by a different action.*/
template <typename T>
class StoreScalar : public Action<T> {
 public:
  virtual ~StoreScalar() {}
  void SetDefault(const T& value) override;
  void SetDestination(T* destination) override;
  std::string GetHelp() const override;

  bool Validate() override;
  void ConsumeArgs(const ParseContext& ctx, std::list<std::string>* args,
                   ActionResult* result) override;

 protected:
  T default_;       //< default value used to initialize the destination
                    //  at the start of parsing, if configured
  T* destination_;  //< pointer to the value assigned during ConsumeArgs
};

// Implements the "store" action for container destinations
/* The "store" action simply parses each argument string into a value of the
 * correct type and appends it to the underlying storage model. */
template <typename T>
class StoreList : public Action<T> {
 public:
  virtual ~StoreList() {}
  void SetDefault(const std::vector<T>&& value) override;
  void SetDestination(
      const std::shared_ptr<StorageModel<T>>& destination) override;

  bool Validate() override;
  void ConsumeArgs(const ParseContext& ctx, std::list<std::string>* args,
                   ActionResult* result) override;

 protected:
  std::vector<T> default_;  //< default list of values value used to initialize
                            //  the destination at the start of parsing, if
                            //  configured

  // pointer to the object which is filled during `ConsumeArgs`
  std::shared_ptr<StorageModel<T>> destination_;
};

// Implements the "store_const" action for scalars.
/* The "store_const" action simply copies some "constant" value into the
 * destination when activated. It does not remove any arguments from the
 * argument list during `ConsumeArgs`. */
template <typename T>
class StoreConst : public StoreScalar<T> {
 public:
  virtual ~StoreConst() {}
  void SetConst(const T& value) override;

  bool Validate() override;
  void ConsumeArgs(const ParseContext& ctx, std::list<std::string>* args,
                   ActionResult* result) override;

 protected:
  // value which is assigned to the `destination_` when this action is
  // processed (via `ConsumeArgs`).
  T const_;
};

// Implements the "help" action, which prints help text and terminates the
// parse loop.
class Help : public Action<void> {
 public:
  virtual ~Help() {}
  std::string GetHelp() const override;
  bool Validate() override;
  void ConsumeArgs(const ParseContext& ctx, std::list<std::string>* args,
                   ActionResult* result) override;
};

// Implements the "version" action, which prints version text and terminates
// the parse loop.
class Version : public Action<void> {
 public:
  virtual ~Version() {}
  std::string GetHelp() const override;
  bool Validate() override;
  void ConsumeArgs(const ParseContext& ctx, std::list<std::string>* args,
                   ActionResult* result) override;
};

// Optional parameters provided to `AddSubparsers`.
struct SubparserOptions {
  std::string help;  //< help text used to describe the subparsers
};

// Implements the subparser action, which dispatches another parser
/* Subparser actions act like the "store" action for a string type, in the
 * sense that they consume one argument (the subcommand) and store it in a
 * string. They internally store a map of `string` (command name) -> `Parser`
 * and when activated they call `ParseArgs` on whatever `Parser` was mapped
 * to the provided command. They pass the remaining argument list to the
 * subparser.
 *
 * Note that, unlike other actions, this object is returned by the `Parser`
 * that owns it, so that `AddParser` can be called by the user.
 */
class Subparsers : public StoreScalar<std::string> {
 public:
  // The type of the map that we store, mapping comman names to subparsers
  typedef std::map<std::string, std::shared_ptr<Parser>> MapType;

  // These options are cached by the `Subparsers` object and passed on to
  // each parser which it constructs.
  struct Metadata {
    std::string command_prefix;  //< this string is appended to the start of
                                 //  the usage description. It is intended to
                                 //  contain the command string up to this sub
                                 //  command

    size_t subdepth;  //< how many subparsers exist between this one
                      //  and the main one
  };

  explicit Subparsers(const Metadata& meta = {});
  virtual ~Subparsers() {}

  // Recursively validate this action and all subparsers
  bool Validate() override;

  std::string GetHelp() const override;

  // Consume one argument, pass remaining args to appropriate subparser
  /* This action will consume one argument from the argument list. If that
   * argument matches a `command` in the subparser map, then it will pass
   * the remaining arguments to the subparser. Otherwise it is an error and
   * it will throw. */
  void ConsumeArgs(const ParseContext& ctx, std::list<std::string>* args,
                   ActionResult* result) override;

  // Convenience accessor to subparser map iterator
  MapType::const_iterator begin() const;

  // Convenience accessor to subparser map iterator
  MapType::const_iterator end() const;

  // Add a new subparser associated with the given command
  /* This call will construct a new parser object using the provided options,
   * and it will store a pointer to that parser in a map associating it with
   * `command`.
   *
   * When this action is activated it will consume one argument from the
   * argument list. If that argument matches `command`, then it will pass
   * the remaining arguments to the subparser. */
  std::shared_ptr<Parser> AddParser(const std::string& command,
                                    const SubparserOptions& opts = {});

 private:
  MapType subparser_map_;  //< maps command names to parser objects
  Metadata metadata_;      //< cache of common options used for all subparsers
                           // constructed
};

// =============================================================================
//                              KWargs
// =============================================================================

// Provide keyword-argument assignment syntax to the various setters of actions
/* TODO(josh): Make field classes non-constructable outside the KWargs class.
 * NOTE(josh): we need to delete assignment operators (here an below) so
 * that the compiler doesn't automatically match the copy assignment
 * operator for argument types that are construction-convertable to this
 * type */
template <typename T>
class KWargs {
 public:
  class ActionField : public std::shared_ptr<Action<T>> {
   public:
    ActionField();
    ActionField(
        const std::shared_ptr<Action<T>>& action);  // NOLINT(runtime/explicit)
    ActionField(const char* named_action);          // NOLINT(runtime/explicit)
    ActionField(const std::string& named_action);   // NOLINT(runtime/explicit)

    ActionField& operator=(const ActionField&) = delete;
    void operator=(const std::shared_ptr<Action<T>>& action);
    void operator=(const char* named_action);
    void operator=(const std::string& named_action);
  };

  class NargsField {
   public:
    NargsField() {}
    NargsField(int value);        // NOLINT(runtime/explicit)
    NargsField(const char* str);  // NOLINT(runtime/explicit)
    NargsField(char c);           // NOLINT(runtime/explicit)

    NargsField& operator=(const NargsField&) = delete;
    void operator=(int value);
    void operator=(const char* str);
    void operator=(char c);
  };

  class ConstField {
   public:
    ConstField() {}
    ConstField(const T& value);  // NOLINT(runtime/explicit)

    ConstField& operator=(const ConstField&) = delete;
    void operator=(const T& value);
  };

  class DefaultField {
   public:
    DefaultField() {}
    DefaultField(const T& value);  // NOLINT(runtime/explicit)
    // TODO(josh): this is just here so that when T is std::string we can
    // use const char* instead. There's gotta be a better way to get that.
    DefaultField(const char* value);  // NOLINT(runtime/explicit)

    DefaultField& operator=(const DefaultField&) = delete;
    void operator=(const T& value);
    void operator=(const char* value);
  };

  class ChoicesField {
   public:
    ChoicesField() {}
    ChoicesField(
        const std::initializer_list<T>& choices);  // NOLINT(runtime/explicit)

    ChoicesField& operator=(const ChoicesField&) = delete;
    void operator=(const std::initializer_list<T>& choices);
  };

  class DestinationField {
   public:
    DestinationField() {}
    DestinationField(T* destination);  // NOLINT(runtime/explicit)
    template <class Allocator>
    DestinationField(
        std::list<T, Allocator>* destination);  // NOLINT(runtime/explicit)
    template <class Allocator>
    DestinationField(
        std::vector<T, Allocator>* destination);  // NOLINT(runtime/explicit)

    DestinationField& operator=(const DestinationField&) = delete;
    void operator=(T* destination);
    template <class Allocator>
    void operator=(std::list<T, Allocator>* destination);
    template <class Allocator>
    void operator=(std::vector<T, Allocator>* destination);
  };

  class RequiredField {
   public:
    RequiredField() {}
    RequiredField(bool value);  // NOLINT(runtime/explicit)

    RequiredField& operator=(const RequiredField&) = delete;
    void operator=(bool value);
  };

  class HelpField {
   public:
    HelpField() {}
    HelpField(const std::string& value);  // NOLINT(runtime/explicit)
    HelpField(const char* value);         // NOLINT(runtime/explicit)

    HelpField& operator=(const HelpField&) = delete;
    void operator=(const std::string& value);
    void operator=(const char* value);
  };

  class MetavarField {
   public:
    MetavarField() {}
    MetavarField(const std::string& value);  // NOLINT(runtime/explicit)
    MetavarField(const char* value);         // NOLINT(runtime/explicit)

    MetavarField& operator=(const MetavarField&) = delete;
    void operator=(const std::string& value);
    void operator=(const char* value);
  };

  ActionField action;
  NargsField nargs;
  ConstField const_;
  DefaultField default_;
  ChoicesField choices;
  DestinationField dest;
  RequiredField required;
  HelpField help;
  MetavarField metavar;
};

template <>
class KWargs<bool> {
 public:
  class ActionField : public std::shared_ptr<Action<bool>> {
   public:
    ActionField();
    ActionField(const std::shared_ptr<Action<bool>>&
                    action);                       // NOLINT(runtime/explicit)
    ActionField(const char* named_action);         // NOLINT(runtime/explicit)
    ActionField(const std::string& named_action);  // NOLINT(runtime/explicit)

    ActionField& operator=(const ActionField&) = delete;
    void operator=(const std::shared_ptr<Action<bool>>& action);
    void operator=(const char* named_action);
    void operator=(const std::string& named_action);
  };

  class NargsField {
   public:
    NargsField() {}
    NargsField(int value);        // NOLINT(runtime/explicit)
    NargsField(const char* str);  // NOLINT(runtime/explicit)
    NargsField(char c);           // NOLINT(runtime/explicit)

    NargsField& operator=(const NargsField&) = delete;
    void operator=(int value);
    void operator=(const char* str);
    void operator=(char c);
  };

  class ConstField {
   public:
    ConstField() {}
    ConstField(bool value);  // NOLINT(runtime/explicit)

    ConstField& operator=(const ConstField&) = delete;
    void operator=(bool value);
  };

  class DefaultField {
   public:
    DefaultField() {}
    DefaultField(bool value);  // NOLINT(runtime/explicit)

    DefaultField& operator=(const DefaultField&) = delete;
    void operator=(bool value);
  };

  class DestinationField {
   public:
    DestinationField() {}
    DestinationField(bool* destination);  // NOLINT(runtime/explicit)
    template <class Allocator>
    DestinationField(
        std::list<bool, Allocator>* destination);  // NOLINT(runtime/explicit)
    template <class Allocator>
    DestinationField(
        std::vector<bool, Allocator>* destination);  // NOLINT(runtime/explicit)

    DestinationField& operator=(const DestinationField&) = delete;
    void operator=(bool* destination);
    template <class Allocator>
    void operator=(std::list<bool, Allocator>* destination);
    template <class Allocator>
    void operator=(std::vector<bool, Allocator>* destination);
  };

  class RequiredField {
   public:
    RequiredField() {}
    RequiredField(bool value);  // NOLINT(runtime/explicit)

    RequiredField& operator=(const RequiredField&) = delete;
    void operator=(bool value);
  };

  class HelpField {
   public:
    HelpField() {}
    HelpField(const std::string& value);  // NOLINT(runtime/explicit)
    HelpField(const char* value);         // NOLINT(runtime/explicit)

    HelpField& operator=(const HelpField&) = delete;
    void operator=(const std::string& value);
    void operator=(const char* value);
  };

  class MetavarField {
   public:
    MetavarField() {}
    MetavarField(const std::string& value);  // NOLINT(runtime/explicit)
    MetavarField(const char* value);         // NOLINT(runtime/explicit)

    MetavarField& operator=(const MetavarField&) = delete;
    void operator=(const std::string& value);
    void operator=(const char* value);
  };

  ActionField action;
  NargsField nargs;
  ConstField const_;
  DefaultField default_;
  DestinationField dest;
  RequiredField required;
  HelpField help;
  MetavarField metavar;
};

template <>
class KWargs<void> {
 public:
  class ActionField : public std::shared_ptr<Action<void>> {
   public:
    ActionField() {}
    ActionField(const std::shared_ptr<Action<void>>&
                    action);                       // NOLINT(runtime/explicit)
    ActionField(const char* named_action);         // NOLINT(runtime/explicit)
    ActionField(const std::string& named_action);  // NOLINT(runtime/explicit)

    ActionField& operator=(const ActionField&) = delete;
    void operator=(const std::shared_ptr<Action<void>>& action);
    void operator=(const char* named_action);
    void operator=(const std::string& named_action);
  };

  class HelpField {
   public:
    HelpField() {}
    HelpField(const std::string& value);  // NOLINT(runtime/explicit)
    HelpField(const char* value);         // NOLINT(runtime/explicit)

    HelpField& operator=(const HelpField&) = delete;
    void operator=(const std::string& value);
    void operator=(const char* value);
  };

  class MetavarField {
   public:
    MetavarField() {}
    MetavarField(const std::string& value);  // NOLINT(runtime/explicit)
    MetavarField(const char* value);         // NOLINT(runtime/explicit)

    MetavarField& operator=(const MetavarField&) = delete;
    void operator=(const std::string& value);
    void operator=(const char* value);
  };

  ActionField action;
  HelpField help;
  MetavarField metavar;
};

// =============================================================================
//                             Parser Utils
// =============================================================================

// Number of character columns width for each of the three columns of help
// text: 1. short flag, 2. long flag, 3. description
typedef std::array<size_t, 3> ColumnSpec;
extern const ColumnSpec kDefaultColumns;

// Create a string formed by repeating `bit` for `n` times.
std::string Repeat(const std::string bit, int n);

// Wrap the given text to the specified line length
std::string Wrap(const std::string text, size_t line_length = 80);

// Help entry for a flag argument
struct FlagHelp {
  std::string short_flag;              //< two character version, like '-h'
  std::string long_flag;               //< dash-dash version, like '--help'
  std::shared_ptr<ActionBase> action;  //< action associated with these flags
};

// Help entry for a positional argument
struct PositionalHelp {
  std::string name;  //< argument name, used as default metavar and as
                     //  an indicator in any error messages
  std::shared_ptr<ActionBase> action;  //< action associated with this argument
};

// Construct a usage string for a flag argument.
/* Generates a string like "--foo <FOO>" */
std::string GetFlagUsage(const std::string& short_flag,
                         const std::string& long_flag,
                         const std::shared_ptr<ActionBase>& action);

// Construct a usage string for a positional argument.
/* Generates a string like "<FOO>" */
std::string GetPositionalUsage(const std::string& name,
                               const std::shared_ptr<ActionBase>& options);

// Value type for flag maps, allows us to reverse loop up in each list.
struct FlagStore {
  std::string short_flag;  //< The short flag for this action, if it exists
  std::string long_flag;   //< The long flag for this action, if it exists
  std::shared_ptr<ActionBase> action;  //< the action associated with the flag
};

// =============================================================================
//                                 Parser
// =============================================================================

// Main class for parsing command line arguments.
/* Use `AddArgument` to add actions (flags, positionals) to the parser, then
 * call `ParseArgs`. */
class Parser {
 public:
  // Collection of program metadata, used to initialize a parser.
  struct Metadata {
    bool add_help;               //< if true, add default `-h/--help` flags
    bool add_version;            //< if true, add default `-v/--version` flags
    std::string name;            //< the name of the program
    std::list<int> version;      //< the program version number
    std::string author;          //< program author
    std::string copyright;       //< copyright statement
    std::string prolog;          //< help text printed before argument help
    std::string epilog;          //< help text printed after argument help
    std::string command_prefix;  //< used to forward down to subparsers
    size_t subdepth;             //< number of parsers between this one and the
                                 //  main one
  };

  // Construct a new parser.
  /* See the documentation for `Metadata` for the list of all optional
   * construction arguments */
  Parser(const Metadata& meta = {
             /*.add_help=*/true,
             /*.add_version=*/true});  // NOLINT(runtime/explicit)

  // Add a flag argument with the given short and log flag names
  template <typename T = void>
  void AddArgument(const std::string& short_flag, const std::string& long_flag,
                   KWargs<T> spec = {});

  // Add a flag argument with the given short and log flag names
  /* This template exists so that one can avoid specfying the template
     parameter explicitly. The template parameter <T> will be inferred by
     the type of the destination pointer.*/
  template <typename T>
  void AddArgument(const std::string& short_flag, const std::string& long_flag,
                   T* dest, KWargs<typename ElementType<T>::value> spec = {});

  // Add a positional argument or a flag argument that has either a short flag
  // or a long flag but not both.
  template <typename T = void>
  void AddArgument(const std::string& name_or_flag, KWargs<T> spec = {});

  // Add a flag or positional argument with the given name
  /* This template exists so that one can avoid specfying the template
     parameter explicitly. The template parameter <T> will be inferred by
     the type of the destination pointer.*/
  template <typename T>
  void AddArgument(const std::string& name_or_flag, T* dest,
                   KWargs<typename ElementType<T>::value> spec = {});

  // Create the subparser action and return a handle to it. Use this handle
  // to add subparsers dispatched depending on the value of a string argument.
  std::shared_ptr<Subparsers> AddSubparsers(const std::string& name,
                                            std::string* dest,
                                            const SubparserOptions& opts = {});

  // Parse command line out of a standard string vector, as expected in
  // `int main(int argc, char** argv)`.
  int ParseArgs(int argc, char** argv, std::ostream* log = &std::cerr);

  // Parse command line out of a list of strings. This is useful mostly for
  // testing/verification.
  int ParseArgs(const std::initializer_list<std::string>& init_list,
                std::ostream* log = &std::cerr);

  // Parse command line arguments out of a list of string. Arguments are
  // removed from the list, modifying it as the parser works through it's
  // state machine.
  int ParseArgs(std::list<std::string>* args, std::ostream* log = &std::cerr);

  // Print the formatted usage string: the short specification usally printed
  // on command line error or at the top of the help output. This is the the
  // description of generally how to formulate a command call.
  void PrintUsage(std::ostream* out, size_t width = 80);

  // Collection of options for help printing
  struct HelpOptions {
    ColumnSpec columns;  //< specify output colums
    int depth;           //< depth of the print, for recursive cases
  };

  // Print formatted multi-column help specification to the given stream.
  // This is the detailed description usually presented with `-h` or `--help`
  // that lists out all the command line options along with a sentence or
  // paragraph about what the option does.
  void PrintHelp(std::ostream* out,
                 const HelpOptions& opts = {kDefaultColumns, 0});

  // Print the version string to the given stream;
  void PrintVersion(std::ostream* out,
                    const ColumnSpec& columns = kDefaultColumns);

  // Backend for ParseArgs above. The only difference between the two is that//
  // ParseArgs() will swallow any exceptions thrown during parsing while this
  // function will allow them past.
  int ParseArgsImpl(std::list<std::string>* args, std::ostream* out);

  // Return the proglog for the parser help. Primarily used by subcommands for
  // subcommand indexing.
  const std::string& GetProlog();

  // Calls action->Validate() for all positional and flag actions registered
  // to the parser.
  void Validate();

 private:
  Metadata meta_;

  // Mapping of short flag strings (i.e. `-h` or `-v`) to the action associated
  // with them.
  std::map<std::string, FlagStore> short_flags_;

  // Mapping of long flag strings (i.e. `--help` or `--version`) to the action
  // associated with them.
  std::map<std::string, FlagStore> long_flags_;

  // List of actions associated with positional arguments.
  std::list<std::shared_ptr<ActionBase>> positionals_;

  // A list of flag help specifications, in the order which the flags were
  // registered with the parser. This list is what is used by the printer
  // printing help output.
  std::list<FlagHelp> flag_help_;

  // A list of positional argument help specifications, in the order in which
  // the positional arguments where registered with the parser. This list is
  // used by the printer when printing help.
  std::list<PositionalHelp> positional_help_;

  // A list of subcommand help parsers so that we an recurse on sub commands
  std::list<std::shared_ptr<Subparsers>> subcommand_help_;
};

}  // namespace argue

//
//
//
//
//
//
//
//
// =============================================================================
//                       Template Implementations
// =============================================================================
//
//
//
//
//
//
//
//

namespace argue {

template <typename T>
bool HasChoice(const std::vector<T>& choices, const T& query) {
  for (const T& choice : choices) {
    if (query == choice) {
      return true;
    }
  }
  return false;
}

template <typename Container>
std::string Join(const Container& container, const std::string& delim) {
  auto iter = container.begin();
  if (iter == container.end()) {
    return "";
  }

  std::stringstream out;
  out << *(iter++);
  while (iter != container.end()) {
    out << delim << *(iter++);
  }
  return out.str();
}

template <typename Container>
std::vector<typename Container::key_type> Keys(const Container& container) {
  std::vector<typename Container::key_type> out;
  out.reserve(container.size());
  for (auto& pair : container) {
    out.emplace_back(pair.first);
  }
  return out;
}

template <typename Container>
typename Container::value_type ContainerSum(const Container& container) {
  typename Container::value_type result = 0;
  for (auto x : container) {
    result += x;
  }
  return result;
}

// =============================================================================
//                              Exceptions
// =============================================================================

template <typename T>
Assertion& Assertion::operator<<(const T& x) {
  if (!expr_) {
    sstream << x;
  }
  return *this;
}

template <typename T>
Assertion& Assertion::operator<<(const T&& x) {
  if (!expr_) {
    sstream << x;
  }
  return *this;
}

// =============================================================================
//                          String Parsing
// =============================================================================

template <typename T>
int ParseSigned(const std::string& str, T* value) {
  *value = 0;

  size_t idx = 0;
  T multiplier = 1;

  if (str[0] == '-') {
    multiplier = -std::pow(10, str.size() - 2);
    ++idx;
  } else {
    multiplier = std::pow(10, str.size() - 1);
  }

  for (; idx < str.size(); idx++) {
    if ('0' <= str[idx] && str[idx] <= '9') {
      *value += multiplier * static_cast<T>(str[idx] - '0');
      multiplier /= 10;
    } else {
      return -1;
    }
  }

  return 0;
}

template <typename T>
int ParseUnsigned(const std::string& str, T* value) {
  *value = 0;

  T multiplier = std::pow(10, str.size() - 1);
  for (size_t idx = 0; idx < str.size(); idx++) {
    if ('0' <= str[idx] && str[idx] <= '9') {
      *value += multiplier * static_cast<T>(str[idx] - '0');
      multiplier /= 10;
    } else {
      return -1;
    }
  }

  return 0;
}

template <typename T>
int ParseFloat(const std::string& str, T* value) {
  *value = 0.0;

  size_t decimal_idx = str.find('.');
  if (decimal_idx == std::string::npos) {
    decimal_idx = str.size();
  }

  int64_t integral_part = 0;
  int64_t fractional_part = 0;
  int64_t denominator = 10;

  size_t idx = 0;
  int32_t multiplier = 1;

  if (str[0] == '-') {
    multiplier = -std::pow(10, decimal_idx - 2);
    ++idx;
  } else {
    multiplier = std::pow(10, decimal_idx - 1);
  }

  for (; idx < decimal_idx; ++idx) {
    if ('0' <= str[idx] && str[idx] <= '9') {
      integral_part += multiplier * static_cast<uint64_t>(str[idx] - '0');
      multiplier /= 10;
    } else {
      return -1;
    }
  }

  if (decimal_idx == str.size()) {
    *value = static_cast<T>(integral_part);
    return 0;
  }

  denominator = std::pow(10, str.size() - decimal_idx);
  if (str[0] == '-') {
    multiplier = -denominator / 10;
  } else {
    multiplier = denominator / 10;
  }

  ++idx;
  for (; idx < str.size(); ++idx) {
    if ('0' <= str[idx] && str[idx] <= '9') {
      fractional_part += multiplier * static_cast<uint64_t>(str[idx] - '0');
      multiplier /= 10;
    } else {
      return -1;
    }
  }

  *value = static_cast<T>(integral_part) +
           static_cast<T>(fractional_part) / static_cast<T>(denominator);
  return 0;
}

template <typename T>
int Parse(const std::string& str, std::shared_ptr<T>* ptr) {
  return -1;
}

template <typename T, class Allocator>
int Parse(const std::string& str, std::list<T, Allocator>* ptr) {
  return -1;
}

template <typename T, class Allocator>
int Parse(const std::string& str, std::vector<T, Allocator>* ptr) {
  return -1;
}

// =============================================================================
//                           Storage Model
// =============================================================================

template <typename T, class Allocator>
ListModel<T, Allocator>::ListModel(std::list<T, Allocator>* dest)
    : dest_(dest) {
  this->type_name_ = type_string<ListModel<T, Allocator>>();
}

template <typename T, class Allocator>
void ListModel<T, Allocator>::Init(size_t /*capacity_hint*/) {
  dest_->clear();
}

template <typename T, class Allocator>
void ListModel<T, Allocator>::Append(const T& value) {
  dest_->emplace_back(value);
}

template <typename T, class Allocator>
VectorModel<T, Allocator>::VectorModel(std::vector<T, Allocator>* dest)
    : dest_(dest) {
  this->type_name_ = type_string<VectorModel<T, Allocator>>();
}

template <typename T, class Allocator>
void VectorModel<T, Allocator>::Init(size_t capacity_hint) {
  dest_->clear();
  dest_->reserve(capacity_hint);
}

template <typename T, class Allocator>
void VectorModel<T, Allocator>::Append(const T& value) {
  dest_->emplace_back(value);
}

// =============================================================================
//                              Actions
// =============================================================================

template <typename T>
Action<T>::Action() : ActionBase{} {
  this->type_name_ = type_string<T>();
}

template <typename T>
void Action<T>::SetConst(const T& value) {
  ARGUE_THROW(CONFIG_ERROR) << "const= is only valid for StoreConst actions";
}

template <typename T>
void Action<T>::SetDefault(const T& value) {
  ARGUE_THROW(CONFIG_ERROR)
      << "You can't assign a scalar default to a list action!";
}

template <typename T>
void Action<T>::SetDefault(const std::vector<T>&& value) {
  ARGUE_THROW(CONFIG_ERROR)
      << "You can't assign a list default to a scalar action!";
}

template <typename T>
void Action<T>::SetChoices(const std::vector<T>&& value) {
  choices_ = std::move(value);
  this->has_choices_ = 1;
}

template <typename T>
void Action<T>::SetDestination(T* destination) {
  ARGUE_THROW(CONFIG_ERROR)
      << "You can't assign a scalar-model to a list action!";
}

template <typename T>
void Action<T>::SetDestination(const std::shared_ptr<StorageModel<T>>& model) {
  ARGUE_THROW(CONFIG_ERROR)
      << "You can't assign a list-model to a scalar action!";
}

template <typename T>
void StoreScalar<T>::SetDefault(const T& value) {
  default_ = value;
  this->has_default_ = 1;
}

template <typename T>
void StoreScalar<T>::SetDestination(T* destination) {
  destination_ = destination;
  this->has_destination_ = 1;
}

template <typename T>
std::string StoreScalar<T>::GetHelp() const {
  std::list<std::string> parts;
  if (!this->choices_.empty()) {
    parts.push_back(fmt::format("[{}]", Join(this->choices_, ", ")));
  }
  if (this->has_help_) {
    parts.push_back(this->help_);
  }
  return Join(parts, "\n");
}

template <typename T>
bool StoreScalar<T>::Validate() {
  ARGUE_ASSERT(CONFIG_ERROR, !this->has_const_)
      << ".const_= is invalid for action type `store`";
  ARGUE_ASSERT(CONFIG_ERROR, this->has_destination_)
      << ".dest= is required for action type `store`";
  ARGUE_ASSERT(CONFIG_ERROR,
               this->nargs_ == ZERO_OR_ONE || this->nargs_ == EXACTLY_ONE)
      << fmt::format("Invalid nargs_={} for non container", this->nargs_);

  // TODO(josh): should we enable this?
  // ARGUE_ASSERT(CONFIG_ERROR, spec.default_.is_set || spec.required_)
  // << `store` action must either be required or have a default value set;

  if (this->nargs_ == ZERO_OR_ONE || this->nargs_ == ZERO_OR_MORE) {
    // TODO(josh): should we require that default is specified if argument
    // is optional? Perhaps we can use a sentinel that says default is already
    // stored in the destination?
    // ARGUE_ASSERT(CONFIG_ERROR, spec.default_.is_set || spec.required_)
    // << `store` action must either be required or have a default value set;
  }

  if (this->has_default_) {
    *this->destination_ = this->default_;
  }
  return true;
}

template <typename T>
void StoreScalar<T>::ConsumeArgs(const ParseContext& ctx,
                                 std::list<std::string>* args,
                                 ActionResult* result) {
  ArgType arg_type = GetArgType(args->front());

  if (arg_type == POSITIONAL) {
    if (Parse(args->front(), this->destination_)) {
      result->code = PARSE_EXCEPTION;
      return;
    }
    if (this->choices_.size() > 0) {
      ARGUE_ASSERT(INPUT_ERROR, HasChoice(this->choices_, *this->destination_))
          << fmt::format("Invalid value '{}' choose from '{}'", args->front());
    }
    args->pop_front();
  } else {
    ARGUE_THROW(INPUT_ERROR) << fmt::format(
        "Expected a value but instead got a flag {}", ctx.arg.c_str());
  }
}

template <typename T>
void StoreList<T>::SetDefault(const std::vector<T>&& value) {
  default_ = value;
  this->has_default_ = 1;
}

template <typename T>
void StoreList<T>::SetDestination(
    const std::shared_ptr<StorageModel<T>>& destination) {
  destination_ = destination;
  this->has_destination_ = 1;
}

template <typename T>
bool StoreList<T>::Validate() {
  ARGUE_ASSERT(CONFIG_ERROR, !this->has_const_)
      << ".const_= is invalid for action type `store`";
  // ARGUE_ASSERT(CONFIG_ERROR, this->has_destination_)
  //     << ".dest= is required for action type `store`";

  // TODO(josh): should we enable this?
  // ARGUE_ASSERT(CONFIG_ERROR, spec.default_.is_set || spec.required_)
  // << `store` action must either be required or have a default value set;

  if (this->nargs_ == ZERO_OR_ONE || this->nargs_ == ZERO_OR_MORE) {
    // TODO(josh): should we require that default is specified if argument
    // is optional? Perhaps we can use a sentinel that says default is already
    // stored in the destination?
    // ARGUE_ASSERT(CONFIG_ERROR, spec.default_.is_set || spec.required_)
    // << `store` action must either be required or have a default value set;
  }

  if (this->has_default_) {
    this->destination_->Init(this->default_.size());
    for (const auto& elem : this->default_) {
      this->destination_->Append(elem);
    }
  }
  return true;
}

template <typename T>
void StoreList<T>::ConsumeArgs(const ParseContext& ctx,
                               std::list<std::string>* args,
                               ActionResult* result) {
  size_t min_args = 0;
  size_t max_args = 0xffff;
  if (this->nargs_ < 1) {
    switch (this->nargs_) {
      case EXACTLY_ONE:
        min_args = 1;
        max_args = 1;
        break;

      case ZERO_OR_ONE:
        min_args = 0;
        max_args = 1;
        break;

      case ONE_OR_MORE:
        min_args = 1;
        break;

      case ZERO_OR_MORE:
        break;

      default:
        ARGUE_THROW(CONFIG_ERROR)
            << fmt::format("Invalid nargs {} for list store", this->nargs_);
    }
  } else {
    min_args = this->nargs_;
    max_args = this->nargs_;
  }

  if (max_args < 0xffff) {
    this->destination_->Init(max_args);
  } else {
    this->destination_->Init(1);
  }

  T value;
  size_t arg_idx = 0;
  for (arg_idx = 0; arg_idx < max_args && !args->empty(); arg_idx++) {
    ArgType arg_type = GetArgType(args->front());
    if (arg_type == POSITIONAL) {
      if (Parse(args->front(), &value)) {
        result->code = PARSE_EXCEPTION;
        return;
      }

      if (this->choices_.size() > 0) {
        ARGUE_ASSERT(INPUT_ERROR, HasChoice(this->choices_, value))
            << fmt::format("Invalid value '{}' choose from '{}'", args->front(),
                           Join(this->choices_));
      }
      args->pop_front();
      if (this->has_destination_) {
        this->destination_->Append(value);
      }

    } else {
      ARGUE_ASSERT(INPUT_ERROR, arg_idx >= min_args)
          << fmt::format("Expected {} arguments but only got {} before flag {}",
                         min_args, arg_idx + 1, ctx.arg.c_str());
    }
  }

  ARGUE_ASSERT(INPUT_ERROR, arg_idx >= min_args)
      << fmt::format("Expected {} arguments but only got {}", min_args,
                     arg_idx + 1, ctx.arg.c_str());
}

template <typename T>
void StoreConst<T>::SetConst(const T& value) {
  const_ = value;
  this->has_const_ = 1;
}

template <typename T>
bool StoreConst<T>::Validate() {
  ARGUE_ASSERT(CONFIG_ERROR, this->has_const_)
      << "const_= is required for action='store_const'";
  ARGUE_ASSERT(CONFIG_ERROR, this->has_destination_)
      << "dest_= is required for action='store_const'";
  ARGUE_ASSERT(CONFIG_ERROR, !this->has_required_ || !this->required_)
      << "required_ may not be true for action='store_const'";
  // ARGUE_ASSERT(spec.default_.is_set)
  // << "default_= is required for action='store_const'";

  if (this->has_default_) {
    *this->destination_ = this->default_;
  }
  return true;
}

template <typename T>
void StoreConst<T>::ConsumeArgs(const ParseContext& ctx,
                                std::list<std::string>* args,
                                ActionResult* result) {
  *this->destination_ = this->const_;
}

// =============================================================================
//                              Kwargs
// =============================================================================

template <typename T>
KWargs<T>::ActionField::ActionField()
    : std::shared_ptr<Action<T>>(std::make_shared<StoreScalar<T>>()) {}

template <typename T>
KWargs<T>::ActionField::ActionField(const std::shared_ptr<Action<T>>& action)
    : std::shared_ptr<Action<T>>(action) {}

template <typename T>
KWargs<T>::ActionField::ActionField(const char* named_action) {
  (*this) = named_action;
}

template <typename T>
void KWargs<T>::ActionField::operator=(
    const std::shared_ptr<Action<T>>& action) {
  (*static_cast<std::shared_ptr<Action<T>>*>(this)) = action;
}

template <typename T>
void KWargs<T>::ActionField::operator=(const char* named_action) {
  std::shared_ptr<Action<T>> action;
  if (strcmp(named_action, "store") == 0) {
    action = std::make_shared<StoreScalar<T>>();
  } else if (strcmp(named_action, "store_const") == 0) {
    action = std::make_shared<StoreConst<T>>();
  } else {
    ARGUE_ASSERT(CONFIG_ERROR, false) << fmt::format(
        "invalid action={} for type={}", named_action, type_string<T>());
  }
  this->swap(action);
}

template <typename T>
void KWargs<T>::ActionField::operator=(const std::string& named_action) {
  (*this) = named_action.c_str();
}

template <typename T>
KWargs<T>::NargsField::NargsField(int value) {
  (*this) = value;
}

template <typename T>
KWargs<T>::NargsField::NargsField(const char* str) {
  if (str && str[0] != '\0') {
    (*this) = str;
  }
}

template <typename T>
KWargs<T>::NargsField::NargsField(char c) {
  (*this) = c;
}

template <typename T>
void KWargs<T>::NargsField::operator=(int value) {
  if (value > 0 || value == ZERO_OR_MORE || value == ONE_OR_MORE ||
      value == REMAINDER) {
    container_of(this, &KWargs<T>::nargs)->action =
        std::make_shared<StoreList<T>>();
  }
  container_of(this, &KWargs<T>::nargs)->action->SetNargs(value);
}

template <typename T>
void KWargs<T>::NargsField::operator=(const char* str) {
  int value = StringToNargs(str);
  ARGUE_ASSERT(CONFIG_ERROR, value != INVALID_NARGS)
      << fmt::format("Invalid nargs {}", str);
  (*this) = value;
}

template <typename T>
void KWargs<T>::NargsField::operator=(char c) {
  int value = StringToNargs(c);
  ARGUE_ASSERT(CONFIG_ERROR, value != INVALID_NARGS)
      << fmt::format("Invalid nargs {}", c);
  (*this) = value;
}

template <typename T>
KWargs<T>::ConstField::ConstField(const T& value) {
  (*this) = value;
}

template <typename T>
void KWargs<T>::ConstField::operator=(const T& value) {
  container_of(this, &KWargs<T>::const_)->action->SetConst(value);
}

template <typename T>
KWargs<T>::DefaultField::DefaultField(const T& value) {
  (*this) = value;
}

template <typename T>
KWargs<T>::DefaultField::DefaultField(const char* value) {
  (*this) = value;
}

template <typename T>
void KWargs<T>::DefaultField::operator=(const T& value) {
  container_of(this, &KWargs<T>::default_)->action->SetDefault(value);
}

template <typename T>
void KWargs<T>::DefaultField::operator=(const char* value) {
  container_of(this, &KWargs<T>::default_)->action->SetDefault(value);
}

template <typename T>
KWargs<T>::ChoicesField::ChoicesField(const std::initializer_list<T>& choices) {
  (*this) = choices;
}

template <typename T>
void KWargs<T>::ChoicesField::operator=(
    const std::initializer_list<T>& choices) {
  std::vector<T> temp = choices;
  container_of(this, &KWargs<T>::choices)->action->SetChoices(std::move(temp));
}

template <typename T>
KWargs<T>::DestinationField::DestinationField(T* destination) {
  (*this) = destination;
}

template <typename T>
template <class Allocator>
KWargs<T>::DestinationField::DestinationField(
    std::list<T, Allocator>* destination) {
  (*this) = destination;
}

template <typename T>
template <class Allocator>
KWargs<T>::DestinationField::DestinationField(
    std::vector<T, Allocator>* destination) {
  (*this) = destination;
}

template <typename T>
void KWargs<T>::DestinationField::operator=(T* destination) {
  container_of(this, &KWargs<T>::dest)->action->SetDestination(destination);
}

template <typename T>
template <class Allocator>
void KWargs<T>::DestinationField::operator=(
    std::vector<T, Allocator>* destination) {
  std::shared_ptr<StorageModel<T>> model =
      std::make_shared<VectorModel<T, Allocator>>(destination);
  container_of(this, &KWargs<T>::dest)->action->SetDestination(model);
}

template <typename T>
template <class Allocator>
void KWargs<T>::DestinationField::operator=(
    std::list<T, Allocator>* destination) {
  std::shared_ptr<StorageModel<T>> model =
      std::make_shared<ListModel<T, Allocator>>(destination);
  container_of(this, &KWargs<T>::dest)->action->SetDestination(model);
}

template <typename T>
KWargs<T>::RequiredField::RequiredField(bool value) {
  (*this) = value;
}

template <typename T>
void KWargs<T>::RequiredField::operator=(bool value) {
  container_of(this, &KWargs<T>::required)->action->SetRequired(value);
}

template <typename T>
KWargs<T>::HelpField::HelpField(const std::string& value) {
  (*this) = value;
}
template <typename T>
KWargs<T>::HelpField::HelpField(const char* value) {
  (*this) = value;
}

template <typename T>
void KWargs<T>::HelpField::operator=(const std::string& value) {
  container_of(this, &KWargs<T>::help)->action->SetHelp(value);
}
template <typename T>
void KWargs<T>::HelpField::operator=(const char* value) {
  container_of(this, &KWargs<T>::help)->action->SetHelp(value);
}

template <typename T>
KWargs<T>::MetavarField::MetavarField(const std::string& value) {
  (*this) = value;
}
template <typename T>
KWargs<T>::MetavarField::MetavarField(const char* value) {
  (*this) = value;
}

template <typename T>
void KWargs<T>::MetavarField::operator=(const std::string& value) {
  container_of(this, &KWargs<T>::metavar)->action->SetMetavar(value);
}
template <typename T>
void KWargs<T>::MetavarField::operator=(const char* value) {
  container_of(this, &KWargs<T>::metavar)->action->SetMetavar(value);
}

template <class Allocator>
KWargs<bool>::DestinationField::DestinationField(
    std::list<bool, Allocator>* destination) {
  (*this) = destination;
}

template <class Allocator>
KWargs<bool>::DestinationField::DestinationField(
    std::vector<bool, Allocator>* destination) {
  (*this) = destination;
}

template <class Allocator>
void KWargs<bool>::DestinationField::operator=(
    std::list<bool, Allocator>* destination) {
  std::shared_ptr<StorageModel<bool>> model =
      std::make_shared<ListModel<bool, Allocator>>(destination);
  container_of(this, &KWargs<bool>::dest)->action->SetDestination(destination);
}

template <class Allocator>
void KWargs<bool>::DestinationField::operator=(
    std::vector<bool, Allocator>* destination) {
  std::shared_ptr<StorageModel<bool>> model =
      std::make_shared<VectorModel<bool, Allocator>>(destination);
  container_of(this, &KWargs<bool>::dest)->action->SetDestination(destination);
}

// =============================================================================
//                              Parser
// =============================================================================

template <typename T>
void Parser::AddArgument(const std::string& short_flag,
                         const std::string& long_flag, KWargs<T> spec) {
  ARGUE_ASSERT(CONFIG_ERROR, short_flag.size() > 0 || long_flag.size() > 0)
      << "Cannot AddArgument with both short_flag='' and long_flag=''";
  spec.action->SetUsage(USAGE_FLAG);

  FlagStore store{
      .short_flag = short_flag, .long_flag = long_flag, .action = spec.action};

  if (long_flag.size() > 0) {
    ARGUE_ASSERT(CONFIG_ERROR, long_flags_.find(long_flag) == long_flags_.end())
        << fmt::format("Duplicate long flag {}", long_flag.c_str());
    long_flags_[long_flag] = store;
  }

  if (short_flag.size() > 0) {
    ARGUE_ASSERT(CONFIG_ERROR,
                 short_flags_.find(short_flag) == short_flags_.end())
        << fmt::format("Duplicate short flag {}", short_flag.c_str());
    short_flags_[short_flag] = store;
  }

  FlagHelp help{
      .short_flag = short_flag, .long_flag = long_flag, .action = spec.action};
  flag_help_.emplace_back(help);
}

template <typename T>
void Parser::AddArgument(const std::string& short_flag,
                         const std::string& long_flag, T* dest,
                         KWargs<typename ElementType<T>::value> kwargs) {
  if (dest) {
    kwargs.dest = dest;
  }

  this->AddArgument<typename ElementType<T>::value>(short_flag, long_flag,
                                                    kwargs);
}

template <typename T>
void Parser::AddArgument(const std::string& name_or_flag, KWargs<T> spec) {
  ARGUE_ASSERT(CONFIG_ERROR, name_or_flag.size() > 0)
      << "Cannot AddArgument with empty name_or_flag string";
  ArgType arg_type = GetArgType(name_or_flag);
  switch (arg_type) {
    case SHORT_FLAG: {
      this->AddArgument<T>(name_or_flag, std::string(""), spec);
      break;
    }

    case LONG_FLAG: {
      this->AddArgument(std::string(""), name_or_flag, spec);
      break;
    }

    case POSITIONAL: {
      spec.action->SetUsage(USAGE_POSITIONAL);
      positionals_.emplace_back(spec.action);

      PositionalHelp help{.name = name_or_flag, .action = spec.action};
      positional_help_.emplace_back(help);
      break;
    }
  }
}

template <typename T>
void Parser::AddArgument(const std::string& name_or_flag, T* dest,
                         KWargs<typename ElementType<T>::value> kwargs) {
  if (dest) {
    kwargs.dest = dest;
  }
  this->AddArgument<typename ElementType<T>::value>(name_or_flag, kwargs);
}

}  // namespace argue

#define ARGUE_EMPTY \
  {}

#define ARGUE_EMPTY1 ARGUE_EMPTY
#define ARGUE_EMPTY2 ARGUE_EMPTY1, ARGUE_EMPTY
#define ARGUE_EMPTY3 ARGUE_EMPTY2, ARGUE_EMPTY
#define ARGUE_EMPTY4 ARGUE_EMPTY3, ARGUE_EMPTY
#define ARGUE_EMPTY5 ARGUE_EMPTY4, ARGUE_EMPTY
#define ARGUE_EMPTY6 ARGUE_EMPTY5, ARGUE_EMPTY
#define ARGUE_EMPTY7 ARGUE_EMPTY6, ARGUE_EMPTY
#define ARGUE_EMPTY8 ARGUE_EMPTY7, ARGUE_EMPTY
