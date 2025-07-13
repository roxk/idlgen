namespace winrt::idlgen
{
	// Make sure we exit directly when we found a template (templates are not supported)
	template <typename... T>
	struct PackedTemplate {};

	template <int I>
	struct IntegerTemplate {};
}
