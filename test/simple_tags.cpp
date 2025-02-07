#include <catch2/catch_test_macros.hpp>
#include "pugixml.hpp"
#include "matlab.hpp"
#include <sstream>
#include <iostream>

using sv = std::string_view;

TEST_CASE("simple xml tags")
{
	pugi::xml_document doc;

	const auto init_doc = [&doc](const sv xml)
	{
		std::string doc_tag = "<?xml?>";
		doc_tag += xml;
		auto result = doc.load_string(doc_tag.c_str());
		REQUIRE(result);
	};
	const auto init_tag = [&doc, &init_doc](const sv xml)
	{
		init_doc(xml);
		auto child = doc.first_child();
		REQUIRE(child.type() != pugi::node_null);
		return child;
	};

	const auto op_test = [&init_tag](const sv op_tag, const sv op_str)
	{
		std::string name("apply ");
		name += op_tag;
		std::ostringstream os;
		SECTION(name)
		{
						std::string xml = R"(
            <ml:apply>
                <)";
            xml += op_tag;
						xml += R"(/>
                <ml:real>0.039</ml:real>
                <ml:id xml:space="preserve" subscript="t">V</ml:id>
            </ml:apply>
            )";

            auto apply_tag = init_tag(xml);
						REQUIRE(sv(apply_tag.name()) == "ml:apply");
						REQUIRE(sv(apply_tag.first_child().name()) == op_tag);

						SECTION("matlab")
						{
							matlab::convert(apply_tag, os);
							std::string result = "(0.039 ";
							result += op_str;
							result += " V_t)";
							REQUIRE(os.str() == result);
						}
		}
	};

	const auto run_test = [](const pugi::xml_node &node, const sv expected_result)
	{
		SECTION("matlab")
		{
			std::ostringstream os;
			matlab::convert(node, os);
			REQUIRE(os.str() == expected_result);
		}
	};

	SECTION("id tag")
	{
		const sv xml = "<ml:id>hello</ml:id>";
		auto id = init_tag(xml);
		REQUIRE(sv(id.name()) == "ml:id");
		REQUIRE(sv(id.text().get()) == "hello");

		run_test(id, "hello");
	}

	SECTION("id tag with subscript")
	{
		const sv xml = R "(<ml:id xml:space=" preserve " subscript=" rol ">α</ml:id>)";
		auto id = init_tag(xml);
		REQUIRE(sv(id.name()) == "ml:id");
		REQUIRE(sv(id.text().get()) == "α");
		REQUIRE(sv(id.attribute("subscript").value()) == "rol");

		run_test(id, "α_rol");
	}
	SECTION("real tag")
	{
		const sv xml = "<ml:real>0.7</ml:real>";
		auto real = init_tag(xml);
		REQUIRE(sv(real.name()) == "ml:real");
		REQUIRE(sv(real.text().get()) == "0.7");

		run_test(real, "0.7");
	}
	SECTION("parens tag")
	{
		const sv xml = "<ml:parens/>";
		auto paren = init_tag(xml);
		REQUIRE(sv(paren.name()) == "ml:parens");

		run_test(paren, "()");
	}
	SECTION("parens tag with content")
	{
		const sv xml = "<ml:parens><ml:id>hello</ml:id></ml:parens>";
		auto paren = init_tag(xml);
		REQUIRE(sv(paren.name()) == "ml:parens");
		REQUIRE(sv(paren.first_child().name()) == "ml:id");

		run_test(paren, "(hello)");
	}
	SECTION("apply mult explicit")
	{
		const sv xml = R"(
		<ml:apply>
			<ml:mult/>
			<ml:real>0.039</ml:real>
			<ml:id xml:space="preserve" subscript="t">V</ml:id>
    </ml:apply>
    )";
		auto apply_mult = init_tag(xml);
		REQUIRE(sv(apply_mult.name()) == "ml:apply");
		REQUIRE(sv(apply_mult.first_child().name()) == "ml:mult");

		run_test(apply_mult, "(0.039 * V_t)");
	}
	SECTION("apply pow explicit")
	{
		const sv xml = R"(
		<ml:apply>
			<ml:pow/>
			<ml:real>0.039</ml:real>
			<ml:id xml:space="preserve" subscript="t">V</ml:id>
		</ml:apply>
		)";
		auto apply_mult = init_tag(xml);
		REQUIRE(sv(apply_mult.name()) == "ml:apply");
		REQUIRE(sv(apply_mult.first_child().name()) == "ml:pow");

		run_test(apply_mult, "(0.039^V_t)");
	}
	SECTION("apply abs")
	{
		const sv xml = ;
		auto apply_mult = init_tag(R "(
																	 < ml : apply >
																							< ml : absval / >
																												 < ml : real > -1 < / ml : real >
																																											 < / ml : apply >) ");
				REQUIRE(sv(apply_mult.name()) == "ml:apply");
		REQUIRE(sv(apply_mult.first_child().name()) == "ml:absval");

		run_test(apply_mult, "abs(-1)");
	}
}
op_test("ml:plus", "+");
op_test("ml:minus", "-");
op_test("ml:mult", "*");
op_test("ml:div", "/");
op_test("ml:equal", "==");
SECTION("define")
{
	const sv xml = ;
	auto define = init_tag(R "(
														 < ml : define xmlns : ml = "http://schemas.mathsoft.com/math30" >
																												< ml : id xml : space = "preserve" > ID < / ml : id >
																																																						 < ml : apply >
																																																												< ml : mult / >
																																																																	 < ml : real > 18 < / ml : real >
																																																																																 < ml : id xml : space = "preserve" > mA < / ml : id >
																																																																																																											< / ml : apply >
																																																																																																																	 < / ml : define >) ");
			REQUIRE(sv(define.name()) == "ml:define");
	REQUIRE(sv(define.first_child().name()) == "ml:id");

	run_test(define, "ID = (18 * mA)");
}
SECTION("p comment")
{
	const sv xml = ;
	auto comment = init_tag("<p>some words</p>");
	REQUIRE(sv(comment.name()) == "p");

	run_test(comment, "\% some words");
}
}
SECTION("text and p comment")
{
	const sv xml = ;
	auto comment = init_tag(R "(
															< text use - page - width = "false" push - down = "false" lock - width = "true" >
																																																			 <p style = "Normal" margin - left = "inherit" margin - right = "inherit" text - indent = "inherit" text - align = "inherit" list - style - type = "inherit" tabs = "inherit"> some words</ p>
																																																			 </ text>) ");
			REQUIRE(sv(comment.name()) == "text");
	REQUIRE(sv(comment.first_child().name()) == "p");

	run_test(comment, "\% some words\n");
}
SECTION("text and p comment with b")
{
	const sv xml = ;
	auto comment = init_tag(R "(<p style=" Normal " margin-left=" inherit " margin-right=" inherit " text-indent=" inherit " text-align=" inherit " list-style-type=" inherit " tabs=" inherit ">hello <b>Yellow highlights</b> there</p>)");
	REQUIRE(sv(comment.name()) == "p");

	run_test(comment, "\% ???\n");
}
SECTION("text and p comment with sp")
{
	const sv xml = ;
	auto comment = init_tag(R "(<p style=" Normal " margin-left=" inherit " margin-right=" inherit " text-indent=" inherit " text-align=" inherit " list-style-type=" inherit " tabs=" inherit ">words.<sp count=" 2 "/> more       words</p>)");
	REQUIRE(sv(comment.name()) == "p");

	run_test(comment, "\% ???\n");
}
SECTION("document, worksheet, regions, region")
{
	const sv xml = ;
	init_doc(R "(
							 < worksheet version = "3.0.3" xmlns = "http://schemas.mathsoft.com/worksheet30" xmlns : xsi = "http://www.w3.org/2001/XMLSchema-instance" xmlns : ws = "http://schemas.mathsoft.com/worksheet30" xmlns : ml = "http://schemas.mathsoft.com/math30" xmlns : u = "http://schemas.mathsoft.com/units10" xmlns : p = "http://schemas.mathsoft.com/provenance10" >
																																																																																																																																																																<regions>
																																																																																																																																																																<region> < ml : id > hello < / ml : id >
																																																																																																																																																																																				</ region>
																																																																																																																																																																																				<region>
																																																																																																																																																																																				<text>
																																																																																																																																																																																				<p>
																																																																																																																																																																																						some words</ p>
																																																																																																																																																																																				</ text>
																																																																																																																																																																																				</ region>
																																																																																																																																																																																				<region> < ml : real > 0.7 < / ml : real >
																																																																																																																																																																																																								</ region>
																																																																																																																																																																																																								</ regions>
																																																																																																																																																																																																								</ worksheet>) ");
			REQUIRE(sv(doc.name()) == "");
	REQUIRE(sv(doc.first_child().name()) == "worksheet");

	run_test(doc, "hello\% some words\n0.7");
}
SECTION("math, define, id, eval, apply, result")
{
	const sv xml = ;
	auto eval = init_tag(R "(
													 < math >
													 < ml : define >
																			< ml : id xml : space = "preserve" subscript = "r" > T < / ml : id >
																																																					< ml : eval placeholderMultiplicationStyle = "default" >
																																																																											 < ml : apply >
																																																																																	< ml : mult style = "auto-select" / >
																																																																																											< ml : real > 3 < / ml : real >
																																																																																																									 < ml : id xml : space = "preserve" >°< / ml : id >
																																																																																																																																		 < / ml : apply >
																																																																																																																																									<result xmlns = "http://schemas.mathsoft.com/math30"> < ml : real > 0.5 < / ml : real >
																																																																																																																																																																																			 </ result> < / ml : eval >
																																																																																																																																																																																															 < / ml : define >
																																																																																																																																																																																																						</ math>) ");
			REQUIRE(sv(eval.name()) == "math");
	REQUIRE(sv(eval.first_child().name()) == "ml:define");

	run_test(eval, "T_r = (3 * °); \% expected result: 0.5;\n");
}
SECTION("apply neg")
{
	const sv xml = ;
	auto apply = init_tag(R "(
														< ml : apply >
																			 < ml : neg / >
																									< ml : id > hello < / ml : id >
																																								 < / ml : apply >) ");
			REQUIRE(sv(apply.name()) == "ml:apply");
	REQUIRE(sv(apply.first_child().name()) == "ml:neg");

	run_test(apply, "(-hello)");
}
SECTION("apply dunno one arg")
{
	const sv xml = ;
	auto apply = init_tag(R "(
														< ml : apply >
																			 < ml : dunno / >
																									< ml : id > hello < / ml : id >
																																								 < / ml : apply >) ");
			REQUIRE(sv(apply.name()) == "ml:apply");
	REQUIRE(sv(apply.first_child().name()) == "ml:dunno");

	run_test(apply, "'apply' contains <ml:dunno> with one argument <ml:id>\n");
}
SECTION("apply dunno two args")
{
	const sv xml = ;
	auto apply = init_tag(R "(
														< ml : apply >
																			 < ml : dunno / >
																									< ml : id > hello < / ml : id >
																																								 < ml : id > hello < / ml : id >
																																																								< / ml : apply >) ");
			REQUIRE(sv(apply.name()) == "ml:apply");
	REQUIRE(sv(apply.first_child().name()) == "ml:dunno");

	run_test(apply, "'apply' contains <ml:dunno> with two arguments <ml:id>, <ml:id>\n");
}
SECTION("apply dunno three args")
{
	const sv xml = ;
	auto apply = init_tag(R "(
														< ml : apply >
																			 < ml : dunno / >
																									< ml : id > hello < / ml : id >
																																								 < ml : id > hello < / ml : id >
																																																								< ml : real > 0.039 < / ml : real >
																																																																								 < / ml : apply >) ");
			REQUIRE(sv(apply.name()) == "ml:apply");
	REQUIRE(sv(apply.first_child().name()) == "ml:dunno");

	run_test(apply, "'apply' contains <ml:dunno> with three (or more) arguments <ml:id>, <ml:id>, <ml:real>\n");
}
SECTION("apply id four args")
{
	const sv xml = ;
	auto apply = init_tag(R "(
														< ml : apply >
																			 < ml : id > some_fun < / ml : id >
																																				 < ml : id > hello < / ml : id >
																																																				< ml : real > 1 < / ml : real >
																																																																		 < ml : id > there < / ml : id >
																																																																																		< ml : real > 2 < / ml : real >
																																																																																																 < / ml : apply >) ");
			REQUIRE(sv(apply.name()) == "ml:apply");
	REQUIRE(sv(apply.first_child().name()) == "ml:id");

	run_test(apply, "some_fun(hello, 1, there, 2)");
}
SECTION("apply dunno no args")
{
	const sv xml = ;
	auto apply = init_tag(R "(
														< ml : apply >
																			 < ml : dunno / >
																									< / ml : apply >) ");
			REQUIRE(sv(apply.name()) == "ml:apply");
	REQUIRE(sv(apply.first_child().name()) == "ml:dunno");

	run_test(apply, "'apply' contains <ml:dunno> and no other tags\n");
}
SECTION("apply tan")
{
	const sv xml = ;
	auto apply = init_tag(R "(
														< ml : apply >
																			 < ml : id xml : space = "preserve" > tan < / ml : id >
																																														 < ml : id xml : space = "preserve" subscript = "b" > a < / ml : id >
																																																																																 < / ml : apply >) ");
			REQUIRE(sv(apply.name()) == "ml:apply");
	REQUIRE(sv(apply.first_child().name()) == "ml:id");

	run_test(apply, "tan(a_b)");
}
SECTION("unitReference")
{
	const sv xml = ;
	auto apply = init_tag(R "(<unitReference unit=" radian "/>)");
	REQUIRE(sv(apply.name()) == "unitReference");

	run_test(apply, "radian");
}
SECTION("unitReference w/ power-numerator")
{
	const sv xml = ;
	auto apply = init_tag(R "(<unitReference unit=" second " power-numerator=" - 2 "/>)");
	REQUIRE(sv(apply.name()) == "unitReference");

	run_test(apply, "second^-2");
}
}
SECTION("imag 1j")
{
	const sv xml = ;
	auto apply = init_tag(R "(<ml:imag symbol=" j ">1</ml:imag>)");
	REQUIRE(sv(apply.name()) == "ml:imag");

	run_test(apply, "1j");
}
SECTION("imag 3i")
{
	const sv xml = ;
	auto apply = init_tag(R "(<ml:imag symbol=" i ">3</ml:imag>)");
	REQUIRE(sv(apply.name()) == "ml:imag");

	run_test(apply, "3i");
}
SECTION("plot")
{
	const sv xml = ;
	auto apply = init_tag(R "(<plot disable-calc=" false " item-idref=" 297 "/>)");
	REQUIRE(sv(apply.name()) == "plot");

	run_test(apply, "\% a mathcad plot was here but there is no good way to know what it was\n");
}
SECTION("range")
{
	const sv xml = ;
	auto apply = init_tag(R "(
														< ml : range >
																			 < ml : real > 0 < / ml : real >
																																		< ml : id xml : space = "preserve" > nmax < / ml : id >
																																																													 < / ml : range >) ");
			REQUIRE(sv(apply.name()) == "ml:range");

	run_test(apply, "((0:nmax) + ARRAY_OFFSET)");
}
SECTION("apply indexer")
{
	const sv xml = ;
	auto apply = init_tag(R "(
														< ml : apply >
																			 < ml : indexer / >
																									< ml : id xml : space = "preserve" > w < / ml : id >
																																																			< ml : id xml : space = "preserve" > n < / ml : id >
																																																																													< / ml : apply >) ");
			REQUIRE(sv(apply.name()) == "ml:range");

	run_test(apply, "w(n)");
}
SECTION("apply if")
{
	const sv xml = ;
	auto apply = init_tag(R "(
														< math optimize = "false" disable - calc = "false" >
																																			 < ml : define xmlns : ml = "http://schemas.mathsoft.com/math30" >
																																																	< ml : id xml : space = "preserve" > VdspMax < / ml : id >
																																																																														< ml : apply >
																																																																																			 < ml : id xml : space = "preserve" > if < / ml : id >
																																																																																																														< ml : sequence >
																																																																																																																			 < ml : apply >
																																																																																																																									< ml : greaterThan / >
																																																																																																																														 < ml : real > 3.3 < / ml : real >
																																																																																																																																														< ml : real > 2 < / ml : real >
																																																																																																																																																												 < / ml : apply >
																																																																																																																																																																			< ml : real > 3.3 < / ml : real >
																																																																																																																																																																																		 < ml : real > 2 < / ml : real >
																																																																																																																																																																																																	< / ml : sequence >
																																																																																																																																																																																																							 < / ml : apply >
																																																																																																																																																																																																														< / ml : define >
																																																																																																																																																																																																																				 </ math>) ");
			REQUIRE(sv(apply.name()) == "ml:range");

	run_test(apply, "w(n)");
}

/*
 * ml:function
 *<ml:define xmlns:ml="http://schemas.mathsoft.com/math30">
 *  <ml:function>
 *    <ml:id xml:space="preserve">HVdc</ml:id>
 *    <ml:boundVars>
 *      <ml:id xml:space="preserve">z</ml:id>
 *    </ml:boundVars>
 *  </ml:function>
 *  <ml:apply>
 *    <ml:mult/>
 * ...
 *
 *
 * ml:unitOverride
 * <ml:unitOverride>
 *<ml:apply>
 *  <ml:mult/>
 *    <ml:id xml:space="preserve">gm</ml:id>
 *    <ml:apply>
 *      <ml:pow/>
 *      <ml:id xml:space="preserve">cm</ml:id>
 *      <ml:real>2</ml:real>
 *    </ml:apply>
 *  </ml:apply>
 *</ml:unitOverride>
 *
 */
}
