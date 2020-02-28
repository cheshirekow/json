# pylint: disable=undefined-variable

add_header_includes(["json/test/stream_gen_test.h"])
add_source_includes([])

decl_json({
    "decltype(TestA::foo)": ["a", "b", "e", "f"],
    "decltype(TestA::bar)": ["c", "d"],
    "TestA::Boz": ["a", "b"],
    "TestA": ["foo", "bar", "boz"],
})

decl_json({
    "decltype(TestB::xbar)": ["a", "b"],
    "TestB::XBaz": ["a", "b"],
    "TestB": ["a", "b", "c", "d", "e", "f", "g",
              "h", "i", "j", "k", "l", "xbar", "xbaz"],
})

decl_json({
    "TestC::C::F": ["g"],
    "TestC::C": ["d", "e", "f"],
    "TestC": ["a", "b", "c"],
})
