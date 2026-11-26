# M1 API baseline

Baseline revision: `e2da473e6dd5637f1f70625738f8378e211a7478` (Miracle v0.5.28), reconstructed from git history after the fact.

Regex-level inventory of exported declarations (declaration headers only; not a compiler-derived ABI), generated from `src/public` at the inspected baseline revision. Class members are not listed.

## `Miracle:Concepts` — `Concepts.cppm`

- [Miracle] `template <class T> concept StringLike = requires(const T &value)`
- [Miracle] `template <class T> concept OptionalLike = requires(const T &value)`
- [Miracle] `template <class T> concept Aggregate = std::is_aggregate_v<T> and not std::is_union_v<T>`
- [Miracle] `template <class T> concept Enum = std::is_enum_v<T>`

## `Miracle:Diagnostic` — `Diagnostic.cppm`

- [Miracle] `struct diagnostics final`
- [Miracle] `inline constexpr struct diagnostics diagnostics`
- [Miracle] `enum class Severity : u8`
- [Miracle] `struct DiagnosticCodeDescriptor final`
- [Miracle] `template <class Code> concept DiagnosticCode = diagnosticCodeValid<std::remove_cvref_t<Code>>()`
- [Miracle] `struct SourceRange final`
- [Miracle] `enum class SpanRole : u8`
- [Miracle] `class DiagnosticSpan final`
- [Miracle] `class Diagnostic final`
- [Miracle] `enum class ColorMode : u8`
- [Miracle] `enum class SourceMode : u8`
- [Miracle] `enum class DetailMode : u8`
- [Miracle] `enum class Presentation : u8`
- [Miracle] `struct RenderOptions final`
- [Miracle] `[[nodiscard]] auto render(const Diagnostic &diagnostic, RenderOptions options =`
- [Miracle] `) -> String`
- [Miracle] `auto render(const Diagnostic &diagnostic, std::ostream &output, RenderOptions options =`
- [Miracle] `) -> void`
- [Miracle] `enum class[[ = diagnostics::prefix(""), = diagnostics::align(3) ]] ContractCode : u8`
- [Miracle] `[[nodiscard]] auto diagnose(const std::contracts::contract_violation &violation) -> Diagnostic`

## `Miracle:Error` — `Error.cppm`

- [Miracle] `struct ErrorDisplayOptions`
- [Miracle] `struct Error`
- [Miracle] `template <typename T> using Result = std::expected<T, Error>`
- [Miracle] `using bail = std::unexpected<Error>`
- [Miracle] `auto todo(std::source_location loc = std::source_location::current()) -> bail`
- [Miracle] `[[nodiscard]] auto diagnose(const Error &error) -> Diagnostic`

## `Miracle:Fs` — `Fs.cppm`

- [Miracle::fs] `class [[nodiscard]] File`
- [Miracle::fs] `struct OpenOptions`
- [Miracle::fs] `[[nodiscard]] auto temporaryDirectory(StringView prefix = "") -> Result<Path>`
- [Miracle::fs] `[[nodiscard]] auto temporaryPath(const Path &parent, StringView prefix = "", StringView suffix = "") -> Result<Path>`
- [Miracle::fs] `[[nodiscard]] auto createDirectory(const Path &parent, StringView name) -> Result<Path>`

## `Miracle:Iter` — `Iter.cppm`

- [Miracle] `struct Infinity final`
- [Miracle] `inline constexpr Infinity infinity`
- [Miracle] `struct SizeHint final`
- [Miracle] `template <class Iteration, class Policy> class ParallelIter`
- [Miracle] `template <std::ranges::view Iteration> class Peekable final`
- [Miracle] `template <std::ranges::view View, class Projection> class Iter final`
- [Miracle] `template <class View> Iter(View) -> Iter<View>`
- [Miracle] `template <class Iteration, class Policy> class ParallelIter final`
- [Miracle] `template <std::ranges::view View, class Projection> [[nodiscard]] constexpr auto Iter<View, Projection>::parallel() && requires std::ranges::random_access_range`
- [Miracle] `template <std::ranges::view View, class Projection> template <class Policy> [[nodiscard]] constexpr auto Iter<View, Projection>::parallel([[maybe_unused]] Polic`
- [Miracle] `template <std::ranges::viewable_range RangeType> requires(not OptionalLike<std::remove_cvref_t<RangeType>>) [[nodiscard]] constexpr auto iter(RangeType &&range)`
- [Miracle] `template <std::integral T> requires(not std::same_as<std::remove_cv_t<T>, bool>) [[nodiscard]] constexpr auto iter(T stop)`
- [Miracle] `template <std::integral Left, std::integral Right> requires CompatibleRangeEndpoints<Left, Right> [[nodiscard]] constexpr auto iter(Left start, Right stop)`
- [Miracle] `template <std::integral Left, std::integral Right, std::integral Step> requires CompatibleRangeEndpoints<Left, Right> and (not std::same_as<std::remove_cv_t<Ste`
- [Miracle] `template <OptionalLike Maybe> [[nodiscard]] constexpr auto iter(Maybe &maybe)`
- [Miracle] `template <OptionalLike Maybe> requires(not std::is_lvalue_reference_v<Maybe>) [[nodiscard]] constexpr auto iter(Maybe &&maybe)`
- [Miracle] `template <class T> [[nodiscard]] constexpr auto empty()`
- [Miracle] `template <class T> [[nodiscard]] constexpr auto once(T value)`
- [Miracle] `template <class Function> [[nodiscard]] constexpr auto onceWith(Function function)`
- [Miracle] `template <class T> [[nodiscard]] constexpr auto repeat(T value)`
- [Miracle] `template <class T> [[nodiscard]] constexpr auto repeatN(T value, usize count)`
- [Miracle] `template <class Function> [[nodiscard]] constexpr auto repeatWith(Function function)`
- [Miracle] `template <class Function> [[nodiscard]] constexpr auto fromFn(Function function)`
- [Miracle] `template <class T, class Function> requires(not OptionalLike<std::remove_cvref_t<T>>) [[nodiscard]] constexpr auto successors(T seed, Function function)`
- [Miracle] `template <OptionalLike Maybe, class Function> [[nodiscard]] constexpr auto successors(Maybe first, Function function)`
- [Miracle] `template <std::ranges::viewable_range First, std::ranges::viewable_range Second> [[nodiscard]] constexpr auto chain(First &&first, Second &&second)`
- [Miracle] `template <std::ranges::viewable_range First, std::ranges::viewable_range Second> [[nodiscard]] constexpr auto zip(First &&first, Second &&second)`
- `template <class Function> class OnceWith`
- `template <class Function> class RepeatWith`
- `template <class Function> class FromFn`
- `template <class Value, class Function> class Successors`
- `template <std::ranges::view View> requires std::ranges::forward_range<View> class Cycle`
- `template <std::ranges::view View, class State, class Function> class Scan`
- `template <std::ranges::view View> class Intersperse`
- `template <std::ranges::view View, class Function> class IntersperseWith`
- `template <std::ranges::view View, class Function, usize N> class MapWindows`
- `template <std::ranges::view View, usize N> class ArrayChunks`
- `template <std::ranges::view Iteration> class Peekable`
- `template <std::ranges::view View, class Projection = std::identity> class Iter`

## `Miracle:Memory` — `Memory.cppm`

- [Miracle::memory] `struct ProcessMemorySnapshot final`
- [Miracle::memory] `[[nodiscard]] auto processMemory() noexcept -> Option<ProcessMemorySnapshot>`

## `Miracle` — `Miracle.cppm`

- `export import Miracle.Numeric;`
- `export import Miracle.Types;`
- `export import Miracle.Range;`
- `export import :Iter;`
- `export import Miracle.StaticString;`
- `export import Miracle.Feature;`
- `export import :Diagnostic;`
- `export import :Error;`
- `export import :Panic;`
- `export import :Fs;`
- `export import Miracle.Meta;`
- `export import :Bitflags;`
- `export import :Debug;`
- `export import Miracle.Build;`
- `export import Miracle.Capabilities;`
- `export import Miracle.FeatureConfiguration;`
- `export import :Concepts;`
- `export import :Profiling;`
- `export import :Memory;`

## `Miracle:Panic` — `Panic.cppm`

- [Miracle] `enum class TraceMode : u8`
- [Miracle] `struct PanicOptions final`
- [Miracle] `[[noreturn]] auto panic(Diagnostic diagnostic, PanicOptions options =`
- [Miracle] `) noexcept -> void`
- [Miracle] `[[noreturn]] auto panic(const Error &error, PanicOptions options =`
- [Miracle] `) noexcept -> void`
- [Miracle] `[[noreturn]] auto panic(StringView message, PanicOptions =`
- [Miracle] `, std::source_location location = std::source_location::current()) noexcept -> void`

## `Miracle:Profiling` — `Profiling.cppm`

- [Miracle::profiling] `enum class[[= debug::derive]] EventKind : u8`
- [Miracle::profiling] `struct ProfileEvent final`
- [Miracle::profiling] `struct ProfileAggregate final`
- [Miracle::profiling] `struct ProfileSnapshot final`
- [Miracle::profiling] `class ProfileSink final`
- [Miracle::profiling] `class Scope final`
- [Miracle::profiling] `[[nodiscard]] auto profileScope(ProfileSink &sink, StringView name, std::source_location location = std::source_location::current()) -> Scope`

## `Miracle.Feature:Catalog` — `feature/Catalog.cppm`

- [Miracle::feature] `template <auto... Features> requires(FeatureValue<Features> and ...) struct Catalog final`
- [Miracle::feature] `template <auto... Features> requires(FeatureValue<Features> and ...) inline constexpr Catalog<Features...> catalog`

## `Miracle.Feature:Descriptor` — `feature/Descriptor.cppm`

- [Miracle::feature] `enum class FeatureKind : u8`
- [Miracle::feature] `struct Version final`
- [Miracle::feature] `enum class Capability : u8`
- [Miracle::feature] `struct CapabilitySet final`
- [Miracle::feature] `[[nodiscard]] consteval auto detectedCapabilities() noexcept -> CapabilitySet`
- [Miracle::feature] `[[nodiscard]] constexpr auto capabilityName(Capability capability) noexcept -> std::string_view`
- [Miracle::feature] `enum class ResolutionError : u8`
- [Miracle::feature] `template <BasicStaticString Text> struct Description final`
- [Miracle::feature] `template <BasicStaticString Text> inline constexpr Description<Text> description`
- [Miracle::feature] `template <u16 Major, u16 Minor = 0, u16 Patch = 0> struct VersionOption final`
- [Miracle::feature] `template <u16 Major, u16 Minor = 0, u16 Patch = 0> inline constexpr VersionOption<Major, Minor, Patch> version`
- [Miracle::feature] `struct BuildOption final`
- [Miracle::feature] `inline constexpr BuildOption build`
- [Miracle::feature] `struct SemanticOption final`
- [Miracle::feature] `inline constexpr SemanticOption semantic`
- [Miracle::feature] `struct DefaultEnabledOption final`
- [Miracle::feature] `inline constexpr DefaultEnabledOption defaultEnabled`
- [Miracle::feature] `template <BasicStaticString Name> struct Dependency final`
- [Miracle::feature] `template <BasicStaticString Name> struct OptionalDependency final`
- [Miracle::feature] `template <BasicStaticString Name> struct Conflict final`
- [Miracle::feature] `template <BasicStaticString Name> struct Implication final`
- [Miracle::feature] `template <Capability Required> struct CapabilityRequirement final`
- [Miracle::feature] `template <Capability Required> inline constexpr CapabilityRequirement<Required> requiresCapability`
- [Miracle::feature] `template <BasicStaticString Name> struct GroupMembership final`
- [Miracle::feature] `template <BasicStaticString Name> inline constexpr GroupMembership<Name> inGroup`
- [Miracle::feature] `struct CustomOptionTag`
- [Miracle::feature] `template <class T> concept DescriptorOption = requires`
- [Miracle::feature] `and std::is_empty_v<T>`
- [Miracle::feature] `template <BasicStaticString Name, DescriptorOption... Options> struct Descriptor final`
- [Miracle::feature] `template <BasicStaticString Name, class... Options> requires(DescriptorOption<std::remove_cvref_t<Options>> and ...) [[nodiscard]] consteval auto define([[maybe`
- [Miracle::feature] `template <class T> concept Feature = requires`
- [Miracle::feature] `template <auto Value> concept FeatureValue = Feature<std::remove_cvref_t<decltype(Value)>>`
- [Miracle::feature] `template <auto Target> requires FeatureValue<Target> inline constexpr Dependency<Target.name> dependsOn`
- [Miracle::feature] `template <BasicStaticString Name> inline constexpr Dependency<Name> dependsOnNamed`
- [Miracle::feature] `template <auto Target> requires FeatureValue<Target> inline constexpr OptionalDependency<Target.name> optionallyDependsOn`
- [Miracle::feature] `template <BasicStaticString Name> inline constexpr OptionalDependency<Name> optionallyDependsOnNamed`
- [Miracle::feature] `template <auto Target> requires FeatureValue<Target> inline constexpr Conflict<Target.name> conflictsWith`
- [Miracle::feature] `template <BasicStaticString Name> inline constexpr Conflict<Name> conflictsWithNamed`
- [Miracle::feature] `template <auto Target> requires FeatureValue<Target> inline constexpr Implication<Target.name> implies`
- [Miracle::feature] `template <BasicStaticString Name> inline constexpr Implication<Name> impliesNamed`
- [Miracle::feature] `template <BasicStaticString Name, auto... Members> requires(FeatureValue<Members> and ...) struct Group final`
- [Miracle::feature] `template <BasicStaticString Name, auto... Members> inline constexpr Group<Name, Members...> group`
- [Miracle::feature] `template <class T> concept FeatureGroup = requires`
- [Miracle::feature] `and not Feature<T>`
- [Miracle::feature] `template <auto Value> concept FeatureGroupValue = FeatureGroup<std::remove_cvref_t<decltype(Value)>>`

## `Miracle.Feature` — `feature/Feature.cppm`

- `export import :Types;`
- `export import :Descriptor;`
- `export import :Set;`
- `export import :Catalog;`
- `export import :Requirement;`

## `Miracle.Feature:Requirement` — `feature/Requirement.cppm`

- [Miracle::feature] `template <class... Required> struct Requirement final`
- [Miracle::feature] `template <class... Required> requires(Feature<std::remove_cvref_t<Required>> and ...) [[nodiscard]] consteval auto require(Required... required) -> Requirement<`
- [Miracle::feature] `template <auto Required, auto Set> requires FeatureValue<Required> consteval auto requireEnabled() -> void`

## `Miracle.Feature:Set` — `feature/Set.cppm`

- [Miracle::feature] `template <BasicStaticString... Names> struct BuildFeatureSet final`
- [Miracle::feature] `template <BasicStaticString... Names> inline constexpr BuildFeatureSet<Names...> buildSet`
- [Miracle::feature] `template <class Set, usize MessageCapacity> struct ResolutionReport final`
- [Miracle::feature] `template <auto... Features> requires(FeatureValue<Features> and ...) struct Catalog`
- [Miracle::feature] `template <class CatalogType> struct FeatureSet final`

## `Miracle.Feature:Types` — `feature/Types.cppm`

- [Miracle::feature] `enum class OptionKind : u8`

## `Miracle.Meta:Access` — `meta/Access.cppm`

- [Miracle::meta] `inline constexpr MetaFunction isAccessible`
- [Miracle::meta] `struct CurrentFunction final`
- [Miracle::meta] `inline constexpr CurrentFunction currentFunction`
- [Miracle::meta] `struct CurrentClass final`
- [Miracle::meta] `inline constexpr CurrentClass currentClass`
- [Miracle::meta] `struct CurrentNamespace final`
- [Miracle::meta] `inline constexpr CurrentNamespace currentNamespace`
- [Miracle::meta] `inline constexpr MetaFunction hasInaccessibleNonstaticDataMembers`
- [Miracle::meta] `inline constexpr MetaFunction hasInaccessibleBases`
- [Miracle::meta] `inline constexpr MetaFunction hasInaccessibleSubobjects`

## `Miracle.Meta:Annotations` — `meta/Annotations.cppm`

- [Miracle::meta] `struct AnnotationsOf final`
- [Miracle::meta] `inline constexpr AnnotationsOf annotationsOf`
- [Miracle::meta] `inline constexpr MetaFunction annotationsOfWithType`
- [Miracle::meta] `[[nodiscard]] consteval auto annotations(Info subject) -> std::span<const Info>`
- [Miracle::meta] `template <class T> [[nodiscard]] consteval auto annotations() -> std::span<const Info>`
- [Miracle::meta] `template <class A> [[nodiscard]] consteval auto annotations(Info subject) -> std::span<const Info>`
- [Miracle::meta] `template <class A, class T> [[nodiscard]] consteval auto annotations() -> std::span<const Info>`
- [Miracle::meta] `inline constexpr Predicate isAnnotation`
- [Miracle::meta] `template <class A> inline constexpr Predicate annotated`

## `Miracle:Bitflags` — `meta/Bitflags.cppm`

- [Miracle] `template <BitflagsConcept B> constexpr auto all() noexcept -> B`
- [Miracle] `template <BitflagsConcept B> constexpr auto has(B flags, B flag) noexcept -> bool`
- `inline constexpr Bitflags bitflags`
- `template <Miracle::BitflagsConcept B> constexpr auto bits(B flags) noexcept`
- `template <Miracle::BitflagsConcept B> constexpr auto operator|(B lhs, B rhs) noexcept -> B`
- `template <Miracle::BitflagsConcept B> constexpr auto operator&(B lhs, B rhs) noexcept -> B`
- `template <Miracle::BitflagsConcept B> constexpr auto operator~(B val) noexcept -> B`
- `template <Miracle::BitflagsConcept B> constexpr auto operator|=(B &lhs, B rhs) noexcept -> B`
- `template <Miracle::BitflagsConcept B> constexpr auto operator&=(B &lhs, B rhs) noexcept -> B`

## `Miracle:Debug` — `meta/Debug.cppm`

- `struct DebugDerive final`
- `struct DebugHide final`
- `struct DebugPrefer final`
- `template <usize Size> struct DebugRename final`
- `inline constexpr DebugDerive derive`
- `inline constexpr DebugHide hide`
- `inline constexpr DebugPrefer prefer`
- `template <usize Size> consteval auto rename(const char (&name)[Size]) -> DebugRename<Size>`
- `template <class T> concept DebuggableAggr = meta::annotated<DebugDerive>(^^T) and not Enum<T>`
- `template <class T> concept DebuggableEnum = meta::annotated<DebugDerive>(^^T) and Enum<T>`
- `template <class T> concept Debuggable = DebuggableAggr<T> or DebuggableEnum<T>`
- `template <Enum T> constexpr auto enumName(T value) -> StringView`

## `Miracle.Meta:Generation` — `meta/Generation.cppm`

- [Miracle::meta] `struct DataMemberSpec final`
- [Miracle::meta] `inline constexpr DataMemberSpec dataMemberSpec`
- [Miracle::meta] `struct DefineAggregate final`
- [Miracle::meta] `inline constexpr DefineAggregate defineAggregate`
- [Miracle::meta] `inline constexpr Predicate isDataMemberSpec`

## `Miracle.Meta:Layout` — `meta/Layout.cppm`

- [Miracle::meta] `inline constexpr MetaFunction offsetOf`
- [Miracle::meta] `inline constexpr MetaFunction sizeOf`
- [Miracle::meta] `inline constexpr MetaFunction alignmentOf`
- [Miracle::meta] `inline constexpr MetaFunction bitSizeOf`

## `Miracle.Meta:Members` — `meta/Members.cppm`

- [Miracle::meta] `inline constexpr MetaFunction templateArgumentsOf`
- [Miracle::meta] `inline constexpr MetaFunction membersOf`
- [Miracle::meta] `inline constexpr MetaFunction basesOf`
- [Miracle::meta] `inline constexpr MetaFunction subobjectsOf`
- [Miracle::meta] `inline constexpr MetaFunction staticDataMembersOf`
- [Miracle::meta] `inline constexpr MetaFunction nonstaticDataMembersOf`
- [Miracle::meta] `inline constexpr MetaFunction enumeratorsOf`
- [Miracle::meta] `inline constexpr MetaFunction parametersOf`
- [Miracle::meta] `inline constexpr MetaFunction variableOf`
- [Miracle::meta] `[[nodiscard]] consteval auto members(Info subject, Access access = Access::current()) -> std::span<const Info>`
- [Miracle::meta] `template <class T> [[nodiscard]] consteval auto members(Access access = Access::current()) -> std::span<const Info>`
- [Miracle::meta] `[[nodiscard]] consteval auto fields(Info subject, Access access = Access::current()) -> std::span<const Info>`
- [Miracle::meta] `template <class T> [[nodiscard]] consteval auto fields(Access access = Access::current()) -> std::span<const Info>`
- [Miracle::meta] `[[nodiscard]] consteval auto staticFields(Info subject, Access access = Access::current()) -> std::span<const Info>`
- [Miracle::meta] `template <class T> [[nodiscard]] consteval auto staticFields(Access access = Access::current()) -> std::span<const Info>`
- [Miracle::meta] `[[nodiscard]] consteval auto functions(Info subject, Access access = Access::current()) -> std::span<const Info>`
- [Miracle::meta] `template <class T> [[nodiscard]] consteval auto functions(Access access = Access::current()) -> std::span<const Info>`
- [Miracle::meta] `[[nodiscard]] consteval auto constructors(Info subject, Access access = Access::current()) -> std::span<const Info>`
- [Miracle::meta] `template <class T> [[nodiscard]] consteval auto constructors(Access access = Access::current()) -> std::span<const Info>`
- [Miracle::meta] `[[nodiscard]] consteval auto bases(Info subject, Access access = Access::current()) -> std::span<const Info>`
- [Miracle::meta] `template <class T> [[nodiscard]] consteval auto bases(Access access = Access::current()) -> std::span<const Info>`
- [Miracle::meta] `[[nodiscard]] consteval auto subobjects(Info subject, Access access = Access::current()) -> std::span<const Info>`
- [Miracle::meta] `template <class T> [[nodiscard]] consteval auto subobjects(Access access = Access::current()) -> std::span<const Info>`
- [Miracle::meta] `[[nodiscard]] consteval auto enumerators(Info subject) -> std::span<const Info>`
- [Miracle::meta] `template <class E> requires std::is_enum_v<E> [[nodiscard]] consteval auto enumerators() -> std::span<const Info>`
- [Miracle::meta] `struct Parameters final`
- [Miracle::meta] `inline constexpr Parameters parameters`
- [Miracle::meta] `struct TemplateArguments final`
- [Miracle::meta] `inline constexpr TemplateArguments templateArguments`

## `Miracle.Meta` — `meta/Meta.cppm`

- `export import :Types;`
- `export import :Storage;`
- `export import :Operators;`
- `export import :Names;`
- `export import :Predicates;`
- `export import :Access;`
- `export import :Members;`
- `export import :Layout;`
- `export import :Annotations;`
- `export import :Values;`
- `export import :Substitution;`
- `export import :Generation;`
- `export import :TypeQueries;`
- `export import :TypeTransforms;`
- `export import :Reflect;`
- `export import :Query;`

## `Miracle.Meta:Names` — `meta/Names.cppm`

- [Miracle::meta] `struct Name final`
- [Miracle::meta] `inline constexpr Name name`
- [Miracle::meta] `struct RequireName final`
- [Miracle::meta] `inline constexpr RequireName requireName`
- [Miracle::meta] `struct DisplayName final`
- [Miracle::meta] `inline constexpr DisplayName displayName`
- [Miracle::meta] `struct Parent final`
- [Miracle::meta] `inline constexpr Parent parent`
- [Miracle::meta] `struct SourceLocation final`
- [Miracle::meta] `inline constexpr SourceLocation sourceLocation`
- [Miracle::meta] `inline constexpr MetaFunction identifierOf`
- [Miracle::meta] `inline constexpr MetaFunction displayStringOf`
- [Miracle::meta] `inline constexpr MetaFunction u8IdentifierOf`
- [Miracle::meta] `inline constexpr MetaFunction u8DisplayStringOf`
- [Miracle::meta] `inline constexpr MetaFunction sourceLocationOf`
- [Miracle::meta] `inline constexpr MetaFunction parentOf`

## `Miracle.Meta:Operators` — `meta/Operators.cppm`

- [Miracle::meta] `inline constexpr MetaFunction operatorOf`
- [Miracle::meta] `inline constexpr MetaFunction symbolOf`
- [Miracle::meta] `inline constexpr MetaFunction u8SymbolOf`

## `Miracle.Meta:Predicates` — `meta/Predicates.cppm`

- [Miracle::meta] `template <class Function> struct Predicate final`
- [Miracle::meta] `template <class Function> Predicate(Function) -> Predicate<Function>`
- [Miracle::meta] `template <class PredicateType> requires MetaPredicate<PredicateType> [[nodiscard]] consteval auto operator!(PredicateType predicate)`
- [Miracle::meta] `template <class Left, class Right> requires(MetaPredicate<Left> and MetaPredicate<Right>) [[nodiscard]] consteval auto operator&&(Left left, Right right)`
- [Miracle::meta] `template <class Left, class Right> requires(MetaPredicate<Left> and MetaPredicate<Right>) [[nodiscard]] consteval auto operator||(Left left, Right right)`
- [Miracle::meta] `inline constexpr Predicate hasIdentifier`
- [Miracle::meta] `inline constexpr Predicate hasParent`
- [Miracle::meta] `inline constexpr Predicate hasTemplateArguments`
- [Miracle::meta] `inline constexpr Predicate isFunction`
- [Miracle::meta] `inline constexpr Predicate isFunctionTemplate`
- [Miracle::meta] `inline constexpr Predicate isField`
- [Miracle::meta] `inline constexpr Predicate isStatic`
- [Miracle::meta] `inline constexpr Predicate isInstanceData`
- [Miracle::meta] `inline constexpr Predicate isConstructor`
- [Miracle::meta] `inline constexpr Predicate isNamespace`
- [Miracle::meta] `inline constexpr Predicate isBase`
- [Miracle::meta] `inline constexpr Predicate isEnumerator`
- [Miracle::meta] `inline constexpr Predicate isVariable`
- [Miracle::meta] `inline constexpr Predicate isTemplate`
- [Miracle::meta] `inline constexpr Predicate isTypeAlias`
- [Miracle::meta] `inline constexpr Predicate isNamespaceAlias`
- [Miracle::meta] `inline constexpr Predicate isConcept`
- [Miracle::meta] `inline constexpr Predicate isPublic`
- [Miracle::meta] `inline constexpr Predicate isProtected`
- [Miracle::meta] `inline constexpr Predicate isPrivate`
- [Miracle::meta] `inline constexpr Predicate isVirtual`
- [Miracle::meta] `inline constexpr Predicate isPureVirtual`
- [Miracle::meta] `inline constexpr Predicate isOverride`
- [Miracle::meta] `inline constexpr Predicate isDeleted`
- [Miracle::meta] `inline constexpr Predicate isDefaulted`
- [Miracle::meta] `inline constexpr Predicate isExplicit`
- [Miracle::meta] `inline constexpr Predicate isNoexcept`
- [Miracle::meta] `inline constexpr Predicate isBitField`
- [Miracle::meta] `inline constexpr Predicate isConst`
- [Miracle::meta] `inline constexpr Predicate isVolatile`
- [Miracle::meta] `inline constexpr Predicate isMutableMember`
- [Miracle::meta] `inline constexpr Predicate isLvalueReferenceQualified`
- [Miracle::meta] `inline constexpr Predicate isRvalueReferenceQualified`
- [Miracle::meta] `inline constexpr Predicate hasStaticStorageDuration`
- [Miracle::meta] `inline constexpr Predicate hasThreadStorageDuration`
- [Miracle::meta] `inline constexpr Predicate hasAutomaticStorageDuration`
- [Miracle::meta] `inline constexpr Predicate hasInternalLinkage`
- [Miracle::meta] `inline constexpr Predicate hasModuleLinkage`
- [Miracle::meta] `inline constexpr Predicate hasExternalLinkage`
- [Miracle::meta] `inline constexpr Predicate hasLinkage`
- [Miracle::meta] `inline constexpr Predicate hasCLanguageLinkage`
- [Miracle::meta] `inline constexpr Predicate isClassMember`
- [Miracle::meta] `inline constexpr Predicate isNamespaceMember`
- [Miracle::meta] `inline constexpr Predicate isNonstaticDataMember`
- [Miracle::meta] `inline constexpr Predicate isStaticMember`
- [Miracle::meta] `inline constexpr Predicate isVariableTemplate`
- [Miracle::meta] `inline constexpr Predicate isClassTemplate`
- [Miracle::meta] `inline constexpr Predicate isAliasTemplate`
- [Miracle::meta] `inline constexpr Predicate isConversionFunctionTemplate`
- [Miracle::meta] `inline constexpr Predicate isOperatorFunctionTemplate`
- [Miracle::meta] `inline constexpr Predicate isLiteralOperatorTemplate`
- [Miracle::meta] `inline constexpr Predicate isConstructorTemplate`
- [Miracle::meta] `inline constexpr Predicate isStructuredBinding`
- [Miracle::meta] `inline constexpr Predicate hasDefaultMemberInitializer`
- [Miracle::meta] `inline constexpr Predicate isConversionFunction`
- [Miracle::meta] `inline constexpr Predicate isOperatorFunction`
- [Miracle::meta] `inline constexpr Predicate isLiteralOperator`
- [Miracle::meta] `inline constexpr Predicate isDefaultConstructor`
- [Miracle::meta] `inline constexpr Predicate isCopyConstructor`
- [Miracle::meta] `inline constexpr Predicate isMoveConstructor`
- [Miracle::meta] `inline constexpr Predicate isAssignment`
- [Miracle::meta] `inline constexpr Predicate isCopyAssignment`
- [Miracle::meta] `inline constexpr Predicate isMoveAssignment`
- [Miracle::meta] `inline constexpr Predicate isDestructor`
- [Miracle::meta] `inline constexpr Predicate isSpecialMemberFunction`
- [Miracle::meta] `inline constexpr Predicate isUserProvided`
- [Miracle::meta] `inline constexpr Predicate isUserDeclared`
- [Miracle::meta] `inline constexpr Predicate isValue`
- [Miracle::meta] `inline constexpr Predicate isObject`
- [Miracle::meta] `inline constexpr Predicate isFinal`
- [Miracle::meta] `inline constexpr Predicate isVarargFunction`
- [Miracle::meta] `inline constexpr Predicate hasDefaultArgument`
- [Miracle::meta] `inline constexpr Predicate isExplicitObjectParameter`
- [Miracle::meta] `inline constexpr Predicate isFunctionParameter`
- [Miracle::meta] `inline constexpr Predicate isType`
- [Miracle::meta] `inline constexpr Predicate isComplete`
- [Miracle::meta] `inline constexpr Predicate isEnumerable`
- [Miracle::meta] `inline constexpr Predicate isClass`
- [Miracle::meta] `inline constexpr Predicate isUnion`
- [Miracle::meta] `inline constexpr Predicate isEnum`
- [Miracle::meta] `inline constexpr Predicate isFunctionType`
- [Miracle::meta] `inline constexpr Predicate isCompleteType`
- [Miracle::meta] `inline constexpr Predicate isEnumerableType`
- [Miracle::meta] `inline constexpr Predicate isStructuralType`
- [Miracle::meta] `inline constexpr Predicate isVoidType`
- [Miracle::meta] `inline constexpr Predicate isNullPointerType`
- [Miracle::meta] `inline constexpr Predicate isIntegralType`
- [Miracle::meta] `inline constexpr Predicate isFloatingPointType`
- [Miracle::meta] `inline constexpr Predicate isArrayType`
- [Miracle::meta] `inline constexpr Predicate isPointerType`
- [Miracle::meta] `inline constexpr Predicate isLvalueReferenceType`
- [Miracle::meta] `inline constexpr Predicate isRvalueReferenceType`
- [Miracle::meta] `inline constexpr Predicate isMemberObjectPointerType`
- [Miracle::meta] `inline constexpr Predicate isMemberFunctionPointerType`
- [Miracle::meta] `inline constexpr Predicate isEnumType`
- [Miracle::meta] `inline constexpr Predicate isUnionType`
- [Miracle::meta] `inline constexpr Predicate isClassType`
- [Miracle::meta] `inline constexpr Predicate isReflectionType`
- [Miracle::meta] `inline constexpr Predicate isReferenceType`
- [Miracle::meta] `inline constexpr Predicate isArithmeticType`
- [Miracle::meta] `inline constexpr Predicate isFundamentalType`
- [Miracle::meta] `inline constexpr Predicate isObjectType`
- [Miracle::meta] `inline constexpr Predicate isScalarType`
- [Miracle::meta] `inline constexpr Predicate isCompoundType`
- [Miracle::meta] `inline constexpr Predicate isMemberPointerType`
- [Miracle::meta] `inline constexpr Predicate isConstType`
- [Miracle::meta] `inline constexpr Predicate isVolatileType`
- [Miracle::meta] `inline constexpr Predicate isTriviallyCopyableType`
- [Miracle::meta] `inline constexpr Predicate isStandardLayoutType`
- [Miracle::meta] `inline constexpr Predicate isEmptyType`
- [Miracle::meta] `inline constexpr Predicate isPolymorphicType`
- [Miracle::meta] `inline constexpr Predicate isAbstractType`
- [Miracle::meta] `inline constexpr Predicate isFinalType`
- [Miracle::meta] `inline constexpr Predicate isAggregateType`
- [Miracle::meta] `inline constexpr Predicate isImplicitLifetimeType`
- [Miracle::meta] `inline constexpr Predicate isSignedType`
- [Miracle::meta] `inline constexpr Predicate isUnsignedType`
- [Miracle::meta] `inline constexpr Predicate isBoundedArrayType`
- [Miracle::meta] `inline constexpr Predicate isUnboundedArrayType`
- [Miracle::meta] `inline constexpr Predicate isScopedEnumType`
- [Miracle::meta] `inline constexpr Predicate isDefaultConstructibleType`
- [Miracle::meta] `inline constexpr Predicate isCopyConstructibleType`
- [Miracle::meta] `inline constexpr Predicate isMoveConstructibleType`
- [Miracle::meta] `inline constexpr Predicate isCopyAssignableType`
- [Miracle::meta] `inline constexpr Predicate isMoveAssignableType`
- [Miracle::meta] `inline constexpr Predicate isSwappableType`
- [Miracle::meta] `inline constexpr Predicate isDestructibleType`
- [Miracle::meta] `inline constexpr Predicate isTriviallyDefaultConstructibleType`
- [Miracle::meta] `inline constexpr Predicate isTriviallyCopyConstructibleType`
- [Miracle::meta] `inline constexpr Predicate isTriviallyMoveConstructibleType`
- [Miracle::meta] `inline constexpr Predicate isTriviallyCopyAssignableType`
- [Miracle::meta] `inline constexpr Predicate isTriviallyMoveAssignableType`
- [Miracle::meta] `inline constexpr Predicate isTriviallyDestructibleType`
- [Miracle::meta] `inline constexpr Predicate isNothrowDefaultConstructibleType`
- [Miracle::meta] `inline constexpr Predicate isNothrowCopyConstructibleType`
- [Miracle::meta] `inline constexpr Predicate isNothrowMoveConstructibleType`
- [Miracle::meta] `inline constexpr Predicate isNothrowCopyAssignableType`
- [Miracle::meta] `inline constexpr Predicate isNothrowMoveAssignableType`
- [Miracle::meta] `inline constexpr Predicate isNothrowSwappableType`
- [Miracle::meta] `inline constexpr Predicate isNothrowDestructibleType`
- [Miracle::meta] `inline constexpr Predicate hasVirtualDestructor`
- [Miracle::meta] `inline constexpr Predicate hasUniqueObjectRepresentations`

## `Miracle.Meta:Query` — `meta/Query.cppm`

- [Miracle::meta] `template <class State> struct Query`
- [Miracle::meta] `template <class State> struct Query final`
- [Miracle::meta] `[[nodiscard]] consteval auto query(const std::vector<Info> &source)`
- [Miracle::meta] `template <class T, std::size_t Extent> [[nodiscard]] consteval auto query(std::span<T, Extent> source)`
- [Miracle::meta] `template <class T, std::size_t N> [[nodiscard]] consteval auto query(std::array<T, N> &source)`
- [Miracle::meta] `template <class T, std::size_t N> [[nodiscard]] consteval auto query(const std::array<T, N> &source)`
- [Miracle::meta] `template <class T, std::size_t N> consteval auto query(std::array<T, N> &&) = delete ( "")`
- [Miracle::meta] `template <class T, std::size_t N> consteval auto query(const std::array<T, N> &&) = delete ( "")`

## `Miracle.Meta:Reflect` — `meta/Reflect.cppm`

- [Miracle] `template <class T> struct Reflect final`
- [Miracle] `template <class T> [[nodiscard]] consteval auto reflect() -> Reflect<T>`

## `Miracle.Meta:Storage` — `meta/Storage.cppm`

- [Miracle::meta] `struct ReflectConstantArray final`
- [Miracle::meta] `inline constexpr ReflectConstantArray reflectConstantArray`
- [Miracle::meta] `struct ReflectConstantString final`
- [Miracle::meta] `inline constexpr ReflectConstantString reflectConstantString`
- [Miracle::meta] `struct DefineStaticArray final`
- [Miracle::meta] `inline constexpr DefineStaticArray defineStaticArray`
- [Miracle::meta] `struct DefineStaticString final`
- [Miracle::meta] `inline constexpr DefineStaticString defineStaticString`
- [Miracle::meta] `struct DefineStaticObject final`
- [Miracle::meta] `inline constexpr DefineStaticObject defineStaticObject`
- [Miracle::meta] `struct IsStringLiteral final`
- [Miracle::meta] `inline constexpr IsStringLiteral isStringLiteral`

## `Miracle.Meta:Substitution` — `meta/Substitution.cppm`

- [Miracle::meta] `struct TemplateOf final`
- [Miracle::meta] `inline constexpr TemplateOf templateOf`
- [Miracle::meta] `struct CanSubstitute final`
- [Miracle::meta] `inline constexpr CanSubstitute canSubstitute`
- [Miracle::meta] `struct Substitute final`
- [Miracle::meta] `inline constexpr Substitute substitute`

## `Miracle.Meta:TypeQueries` — `meta/TypeQueries.cppm`

- [Miracle::meta] `struct Type final`
- [Miracle::meta] `inline constexpr Type type`
- [Miracle::meta] `struct ReturnType final`
- [Miracle::meta] `inline constexpr ReturnType returnType`
- [Miracle::meta] `struct Dealias final`
- [Miracle::meta] `inline constexpr Dealias dealias`
- [Miracle::meta] `inline constexpr MetaFunction typeOf`
- [Miracle::meta] `inline constexpr MetaFunction rank`
- [Miracle::meta] `inline constexpr MetaFunction isAssignableType`
- [Miracle::meta] `inline constexpr MetaFunction isSwappableWithType`
- [Miracle::meta] `inline constexpr MetaFunction isTriviallyAssignableType`
- [Miracle::meta] `inline constexpr MetaFunction isNothrowAssignableType`
- [Miracle::meta] `inline constexpr MetaFunction isNothrowSwappableWithType`
- [Miracle::meta] `inline constexpr MetaFunction referenceConstructsFromTemporary`
- [Miracle::meta] `inline constexpr MetaFunction referenceConvertsFromTemporary`
- [Miracle::meta] `inline constexpr MetaFunction isSameType`
- [Miracle::meta] `inline constexpr MetaFunction typeOrder`
- [Miracle::meta] `inline constexpr MetaFunction isBaseOfType`
- [Miracle::meta] `inline constexpr MetaFunction isVirtualBaseOfType`
- [Miracle::meta] `inline constexpr MetaFunction isConvertibleType`
- [Miracle::meta] `inline constexpr MetaFunction isNothrowConvertibleType`
- [Miracle::meta] `inline constexpr MetaFunction isLayoutCompatibleType`
- [Miracle::meta] `inline constexpr MetaFunction isPointerInterconvertibleBaseOfType`
- [Miracle::meta] `inline constexpr MetaFunction isApplicableType`
- [Miracle::meta] `inline constexpr MetaFunction isNothrowApplicableType`
- [Miracle::meta] `inline constexpr MetaFunction tupleSize`
- [Miracle::meta] `inline constexpr MetaFunction variantSize`
- [Miracle::meta] `inline constexpr MetaFunction returnTypeOf`
- [Miracle::meta] `struct IsConstructibleType final`
- [Miracle::meta] `inline constexpr IsConstructibleType isConstructibleType`
- [Miracle::meta] `struct IsTriviallyConstructibleType final`
- [Miracle::meta] `inline constexpr IsTriviallyConstructibleType isTriviallyConstructibleType`
- [Miracle::meta] `struct IsNothrowConstructibleType final`
- [Miracle::meta] `inline constexpr IsNothrowConstructibleType isNothrowConstructibleType`
- [Miracle::meta] `struct IsInvocableType final`
- [Miracle::meta] `inline constexpr IsInvocableType isInvocableType`
- [Miracle::meta] `struct IsInvocableRType final`
- [Miracle::meta] `inline constexpr IsInvocableRType isInvocableRType`
- [Miracle::meta] `struct IsNothrowInvocableType final`
- [Miracle::meta] `inline constexpr IsNothrowInvocableType isNothrowInvocableType`
- [Miracle::meta] `struct IsNothrowInvocableRType final`
- [Miracle::meta] `inline constexpr IsNothrowInvocableRType isNothrowInvocableRType`
- [Miracle::meta] `struct Extent final`
- [Miracle::meta] `inline constexpr Extent extent`

## `Miracle.Meta:TypeTransforms` — `meta/TypeTransforms.cppm`

- [Miracle::meta] `struct RemoveCv final`
- [Miracle::meta] `inline constexpr RemoveCv removeCv`
- [Miracle::meta] `struct RemoveReference final`
- [Miracle::meta] `inline constexpr RemoveReference removeReference`
- [Miracle::meta] `struct RemoveCvref final`
- [Miracle::meta] `inline constexpr RemoveCvref removeCvref`
- [Miracle::meta] `inline constexpr MetaFunction applyResult`
- [Miracle::meta] `inline constexpr MetaFunction removeConst`
- [Miracle::meta] `inline constexpr MetaFunction removeVolatile`
- [Miracle::meta] `inline constexpr MetaFunction addConst`
- [Miracle::meta] `inline constexpr MetaFunction addVolatile`
- [Miracle::meta] `inline constexpr MetaFunction addCv`
- [Miracle::meta] `inline constexpr MetaFunction addLvalueReference`
- [Miracle::meta] `inline constexpr MetaFunction addRvalueReference`
- [Miracle::meta] `inline constexpr MetaFunction makeSigned`
- [Miracle::meta] `inline constexpr MetaFunction makeUnsigned`
- [Miracle::meta] `inline constexpr MetaFunction removeExtent`
- [Miracle::meta] `inline constexpr MetaFunction removeAllExtents`
- [Miracle::meta] `inline constexpr MetaFunction removePointer`
- [Miracle::meta] `inline constexpr MetaFunction addPointer`
- [Miracle::meta] `inline constexpr MetaFunction decay`
- [Miracle::meta] `inline constexpr MetaFunction unwrapReference`
- [Miracle::meta] `inline constexpr MetaFunction unwrapRefDecay`
- [Miracle::meta] `struct CommonType final`
- [Miracle::meta] `inline constexpr CommonType commonType`
- [Miracle::meta] `struct CommonReference final`
- [Miracle::meta] `inline constexpr CommonReference commonReference`
- [Miracle::meta] `struct UnderlyingType final`
- [Miracle::meta] `inline constexpr UnderlyingType underlyingType`
- [Miracle::meta] `struct InvokeResult final`
- [Miracle::meta] `inline constexpr InvokeResult invokeResult`
- [Miracle::meta] `struct TupleElement final`
- [Miracle::meta] `inline constexpr TupleElement tupleElement`
- [Miracle::meta] `struct VariantAlternative final`
- [Miracle::meta] `inline constexpr VariantAlternative variantAlternative`

## `Miracle.Meta:Types` — `meta/Types.cppm`

- [Miracle] `using Access = std::meta::access_context`
- [Miracle::meta] `using Info = std::meta::info`
- [Miracle::meta] `using exception = std::meta::exception`
- [Miracle::meta] `using Exception = exception`
- [Miracle::meta] `using AccessContext = std::meta::access_context`
- [Miracle::meta] `template <class Range> concept ReflectionRange = std::meta::reflection_range<Range>`
- [Miracle::meta] `using Operators = std::meta::operators`
- [Miracle::meta] `using DataMemberOptions = std::meta::data_member_options`
- [Miracle::meta] `using MemberOffset = std::meta::member_offset`
- [Miracle::meta] `template <class Function> struct MetaFunction final`
- [Miracle::meta] `template <class Function> MetaFunction(Function) -> MetaFunction<Function>`
- [Miracle::meta] `struct Raw final`
- [Miracle::meta] `inline constexpr Raw raw`

## `Miracle.Meta:Values` — `meta/Values.cppm`

- [Miracle::meta] `struct Constant final`
- [Miracle::meta] `inline constexpr Constant constant`
- [Miracle::meta] `template <class T> struct Extract final`
- [Miracle::meta] `template <class T> inline constexpr Extract<T> extract`
- [Miracle::meta] `inline constexpr MetaFunction objectOf`
- [Miracle::meta] `inline constexpr MetaFunction constantOf`
- [Miracle::meta] `struct ReflectObject final`
- [Miracle::meta] `inline constexpr ReflectObject reflectObject`
- [Miracle::meta] `struct ReflectFunction final`
- [Miracle::meta] `inline constexpr ReflectFunction reflectFunction`

## `Miracle.Numeric` — `numeric/Numeric.cppm`

- [Miracle] `using u8 = std::uint8_t`
- [Miracle] `using u16 = std::uint16_t`
- [Miracle] `using u32 = std::uint32_t`
- [Miracle] `using u64 = std::uint64_t`
- [Miracle] `using i8 = std::int8_t`
- [Miracle] `using i16 = std::int16_t`
- [Miracle] `using i32 = std::int32_t`
- [Miracle] `using i64 = std::int64_t`
- [Miracle] `using f32 = float`
- [Miracle] `using f64 = double`
- [Miracle] `using usize = std::size_t`
- [Miracle] `using isize = std::ptrdiff_t`

## `Miracle.Range` — `range/Range.cppm`

- [Miracle] `template <class T> concept RangeElement = std::integral<T> and std::same_as<T, std::remove_cv_t<T>> and not std::same_as<T, bool>`
- [Miracle] `template <class Left, class Right> concept CompatibleRangeEndpoints = RangeElement<Left> and RangeElement<Right> and (std::is_signed_v<Left> == std::is_signed_v`
- [Miracle] `template <class T> requires RangeElement<T> struct Range final`
- [Miracle] `template <RangeElement T> Range(T) -> Range<T>`
- [Miracle] `template <class Left, class Right> requires CompatibleRangeEndpoints<Left, Right> Range(Left, Right) -> Range<std::common_type_t<Left, Right>>`

## `Miracle.StaticString` — `static_string/StaticString.cppm`

- [Miracle] `template <class Char, usize Capacity> struct StaticStringSplit`
- [Miracle] `template <class Char, usize Capacity> struct BasicStaticString final`
- [Miracle] `template <class Char, usize N> BasicStaticString(const Char (&)[N]) -> BasicStaticString<Char, N - 1>`
- [Miracle] `template <usize Capacity> using StaticString = BasicStaticString<char, Capacity>`
- [Miracle] `template <class Char, usize Capacity> struct StaticStringSplit final`
- [Miracle] `template <class Char, usize Capacity> constexpr auto BasicStaticString<Char, Capacity>::split(Char delimiter) const noexcept -> StaticStringSplit<Char, Capacity`
- [Miracle] `template <class Char, usize Capacity> template <usize DelimiterCapacity> constexpr auto BasicStaticString<Char, Capacity>::split( const BasicStaticString<Char, `
- [Miracle] `template <class Char, usize LeftCapacity, usize RightCapacity> [[nodiscard]] constexpr auto operator+(const BasicStaticString<Char, LeftCapacity> &left, const B`
- [Miracle] `template <class Char, usize LeftCapacity, usize RightCapacity> [[nodiscard]] constexpr auto operator==(const BasicStaticString<Char, LeftCapacity> &left, const `
- [Miracle] `template <class Char, usize LeftCapacity, usize RightCapacity> [[nodiscard]] constexpr auto operator<=>(const BasicStaticString<Char, LeftCapacity> &left, const`
- [Miracle] `template <class Char, usize Capacity> [[nodiscard]] constexpr auto operator==(const BasicStaticString<Char, Capacity> &left, std::basic_string_view<Char> right)`
- [Miracle] `template <class Char, usize Capacity> [[nodiscard]] constexpr auto operator==(std::basic_string_view<Char> left, const BasicStaticString<Char, Capacity> &right)`
- [Miracle] `template <class Char, usize Capacity> [[nodiscard]] constexpr auto operator==(const BasicStaticString<Char, Capacity> &left, const Char *right) noexcept -> bool`
- [Miracle] `template <class Char, usize Capacity> [[nodiscard]] constexpr auto operator==(const Char *left, const BasicStaticString<Char, Capacity> &right) noexcept -> bool`
- [Miracle] `template <class Char, usize Capacity> [[nodiscard]] constexpr auto operator<=>(const BasicStaticString<Char, Capacity> &left, std::basic_string_view<Char> right`
- [Miracle] `template <class Char, usize Capacity> [[nodiscard]] constexpr auto operator<=>(std::basic_string_view<Char> left, const BasicStaticString<Char, Capacity> &right`
- [Miracle] `template <class Char, usize Capacity> auto operator<=>(const BasicStaticString<Char, Capacity> &, const Char *) = delete ( "" "")`
- [Miracle] `template <class Char, usize Capacity> auto operator<=>(const Char *, const BasicStaticString<Char, Capacity> &) = delete ( "" "")`
- [Miracle] `template <class Char, usize SeparatorCapacity, usize FirstCapacity, usize... RestCapacity> [[nodiscard]] constexpr auto join(const BasicStaticString<Char, Separ`
- [Miracle] `template <BasicStaticString Format, class... Args> requires(StaticStringFormatting::Argument<Args> && ...) [[nodiscard]] consteval auto staticFormat(Args &&...a`
- [Miracle] `template <class Char, Miracle::usize Capacity> struct std::formatter<Miracle::BasicStaticString<Char, Capacity>, Char> : std::formatter<std::basic_string_view<C`

## `Miracle.Types` — `types/Types.cppm`

- [Miracle] `using String = std::string`
- [Miracle] `using StringView = std::string_view`
- [Miracle] `using Byte = std::byte`
- [Miracle] `using Path = std::filesystem::path`
- [Miracle] `template <class T, usize N> using Array = std::array<T, N>`
- [Miracle] `template <class T> using Vec = std::vector<T>`
- [Miracle] `template <class T> using UPtr = std::unique_ptr<T>`
- [Miracle] `template <class T> using Ref = std::reference_wrapper<T>`
- [Miracle] `template <class T1, class T2> using Pair = std::pair<T1, T2>`
- [Miracle] `template <class... Values> using Tuple = std::tuple<Values...>`
- [Miracle] `template <class T> using Option = std::optional<T>`
- [Miracle] `inline constexpr std::nullopt_t None = std::nullopt`
- [Miracle] `template <class T, usize Extent = std::dynamic_extent> using Span = std::span<T, Extent>`
- [Miracle] `template <class T, class Compare = std::less<T>> using FlatSet = std::flat_set<T, Compare, Vec<T>>`
- [Miracle] `template <class Key, class Value, class Compare = std::less<Key>> using FlatMap = std::flat_map<Key, Value, Compare, Vec<Key>, Vec<Value>>`
- [Miracle] `template <class T, usize Capacity> using InplaceVec = std::inplace_vector<T, Capacity>`
- [Miracle] `template <class T> using Hive = std::hive<T>`

