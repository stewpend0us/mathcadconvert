#include <catch2/catch_test_macros.hpp>
#include "pugixml.hpp"
#include "matlab.hpp"
#include <sstream>
#include <iostream>

using sv = std::string_view;

TEST_CASE( "simple xml tags" )
{
	pugi::xml_document doc;

    const auto init_doc = [&doc](const sv xml){
        std::string doc_tag = "<?xml?>";
        doc_tag += xml;
        auto result = doc.load_string(doc_tag.c_str());
        REQUIRE( result );
    };
    const auto init_tag = [&doc, &init_doc](const sv xml){
        init_doc(xml);
        auto child = doc.first_child();
        REQUIRE( child.type() != pugi::node_null );
        return child;
    };


    const auto op_test = [&init_tag](const sv op_tag, const sv op_str){
        std::string name("apply ");
        name += op_tag;
        std::ostringstream os;
        SECTION( name )
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
            REQUIRE( sv(apply_tag.name()) == "ml:apply");
            REQUIRE( sv(apply_tag.first_child().name()) == op_tag);

            SECTION( "matlab" )
            {
                matlab::convert(apply_tag, os);
                std::string result = "(0.039 ";
                result += op_str;
                result += " V_t)";
                REQUIRE( os.str() == result );
            }
        }
    };

    std::ostringstream os;

    SECTION( "id tag" )
    {
        auto id = init_tag("<ml:id>hello</ml:id>");
        REQUIRE( sv(id.name()) == "ml:id");
        REQUIRE( sv(id.text().get()) == "hello");

        SECTION( "matlab" )
        {
            matlab::convert(id, os);
            REQUIRE( os.str() == "hello" );
        }
    }
    SECTION( "id tag with subscript" )
    {
        auto id = init_tag(R"(<ml:id xml:space="preserve" subscript="rol">α</ml:id>)");
        REQUIRE( sv(id.name()) == "ml:id");
        REQUIRE( sv(id.text().get()) == "α");
        REQUIRE( sv(id.attribute("subscript").value()) == "rol");

        SECTION( "matlab" )
        {
            matlab::convert(id, os);
            REQUIRE( os.str() == "α_rol" );
        }
    }
    SECTION( "real tag" )
    {
        auto real = init_tag("<ml:real>0.7</ml:real>");
        REQUIRE( sv(real.name()) == "ml:real");
        REQUIRE( sv(real.text().get()) == "0.7");

        SECTION( "matlab" )
        {
            matlab::convert(real, os);
            REQUIRE( os.str() == "0.7" );
        }
    }
    SECTION( "parens tag" )
    {
        auto paren = init_tag("<ml:parens/>");
        REQUIRE( sv(paren.name()) == "ml:parens");

        SECTION( "matlab" )
        {
            matlab::convert(paren, os);
            REQUIRE( os.str() == "()" );
        }
    }
    SECTION( "parens tag with content" )
    {
        auto paren = init_tag("<ml:parens><ml:id>hello</ml:id></ml:parens>");
        REQUIRE( sv(paren.name()) == "ml:parens");
        REQUIRE( sv(paren.first_child().name()) == "ml:id");

        SECTION( "matlab" )
        {
            matlab::convert(paren, os);
            REQUIRE( os.str() == "(hello)" );
        }
    }
    SECTION( "apply mult explicit" )
    {
        auto apply_mult = init_tag(R"(
        <ml:apply>
            <ml:mult/>
            <ml:real>0.039</ml:real>
            <ml:id xml:space="preserve" subscript="t">V</ml:id>
        </ml:apply>
        )");
        REQUIRE( sv(apply_mult.name()) == "ml:apply");
        REQUIRE( sv(apply_mult.first_child().name()) == "ml:mult");

        SECTION( "matlab" )
        {
            matlab::convert(apply_mult, os);
            REQUIRE( os.str() == "(0.039 * V_t)" );
        }

    }
    SECTION( "apply pow explicit" )
    {
        auto apply_mult = init_tag(R"(
        <ml:apply>
            <ml:pow/>
            <ml:real>0.039</ml:real>
            <ml:id xml:space="preserve" subscript="t">V</ml:id>
        </ml:apply>
        )");
        REQUIRE( sv(apply_mult.name()) == "ml:apply");
        REQUIRE( sv(apply_mult.first_child().name()) == "ml:pow");

        SECTION( "matlab" )
        {
            matlab::convert(apply_mult, os);
            REQUIRE( os.str() == "(0.039^V_t)" );
        }

    }
    op_test("ml:plus", "+");
    op_test("ml:minus", "-");
    op_test("ml:mult", "*");
    op_test("ml:div", "/");
    op_test("ml:equal", "==");
    SECTION( "define" )
    {
        auto define = init_tag(R"(
        <ml:define xmlns:ml="http://schemas.mathsoft.com/math30">
            <ml:id xml:space="preserve">ID</ml:id>
            <ml:apply>
                <ml:mult/>
                <ml:real>18</ml:real>
                <ml:id xml:space="preserve">mA</ml:id>
       </ml:apply>
        </ml:define>
        )");
        REQUIRE( sv(define.name()) == "ml:define");
        REQUIRE( sv(define.first_child().name()) == "ml:id");

        SECTION( "matlab" )
        {
            matlab::convert(define, os);
            REQUIRE( os.str() == "ID = (18 * mA)" );
        }
    }
    SECTION( "p comment" )
    {
        auto comment = init_tag("<p>some words</p>");
        REQUIRE( sv(comment.name()) == "p");

        SECTION( "matlab" )
        {
            matlab::convert(comment, os);
            REQUIRE( os.str() == "\% some words" );
        }
    }
    SECTION( "text and p comment" )
    {
        auto comment = init_tag(R"(
		<text use-page-width="false" push-down="false" lock-width="true">
			<p style="Normal" margin-left="inherit" margin-right="inherit" text-indent="inherit" text-align="inherit" list-style-type="inherit" tabs="inherit">some words</p>
		</text>
        )");
        REQUIRE( sv(comment.name()) == "text");
        REQUIRE( sv(comment.first_child().name()) == "p");

        SECTION( "matlab" )
        {
            matlab::convert(comment, os);
            REQUIRE( os.str() == "\% some words\n" );
        }
    }
    SECTION( "document, worksheet, regions, region" )
    {
        init_doc(R"(
        <worksheet version="3.0.3" xmlns="http://schemas.mathsoft.com/worksheet30" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xmlns:ws="http://schemas.mathsoft.com/worksheet30" xmlns:ml="http://schemas.mathsoft.com/math30" xmlns:u="http://schemas.mathsoft.com/units10" xmlns:p="http://schemas.mathsoft.com/provenance10">
            <regions>
                <region>
                    <ml:id>hello</ml:id>
                </region>
                <region>
		            <text>
		            	<p>some words</p>
		            </text>
                </region>
                <region>
                    <ml:real>0.7</ml:real>
                </region>
            </regions>
        </worksheet>
        )");
        REQUIRE( sv(doc.name()) == "");
        REQUIRE( sv(doc.first_child().name()) == "worksheet");

        SECTION( "matlab" )
        {
            matlab::convert(doc, os);
            REQUIRE( os.str() == "hello\% some words\n0.7" );
        }
    }
    SECTION( "math, define, id, eval, apply, result" )
    {
        auto eval = init_tag(R"(
        <math>
		    <ml:define>
		    	<ml:id xml:space="preserve" subscript="r">T</ml:id>
		        <ml:eval placeholderMultiplicationStyle="default">
		        	<ml:apply>
		        		<ml:mult style="auto-select"/>
		        		<ml:real>3</ml:real>
		        		<ml:id xml:space="preserve">°</ml:id>
		        	</ml:apply>
		        	<result xmlns="http://schemas.mathsoft.com/math30">
		        		<ml:real>0.5</ml:real>
		        	</result>
		        </ml:eval>
            </ml:define>
        </math>
        )");
        REQUIRE( sv(eval.name()) == "math");
        REQUIRE( sv(eval.first_child().name()) == "ml:define");

        SECTION( "matlab" )
        {
            matlab::convert(eval, os);
            REQUIRE( os.str() == "T_r = (3 * °); \% expected result: 0.5;\n" );
        }
    }
    SECTION( "apply neg" )
    {
        auto apply = init_tag(R"(
		<ml:apply>
			<ml:neg/>
            <ml:id>hello</ml:id>
		</ml:apply>
        )");
        REQUIRE( sv(apply.name()) == "ml:apply");
        REQUIRE( sv(apply.first_child().name()) == "ml:neg");

        SECTION( "matlab" )
        {
            matlab::convert(apply, os);
            REQUIRE( os.str() == "(-hello)" );
        }
    }
    SECTION( "apply dunno one arg" )
    {
        auto apply = init_tag(R"(
		<ml:apply>
			<ml:dunno/>
            <ml:id>hello</ml:id>
		</ml:apply>
        )");
        REQUIRE( sv(apply.name()) == "ml:apply");
        REQUIRE( sv(apply.first_child().name()) == "ml:dunno");

        SECTION( "matlab" )
        {
            matlab::convert(apply, os);
            REQUIRE( os.str() == "'apply' contains <ml:dunno> with one argument <ml:id>\n" );
        }
    }
    SECTION( "apply dunno two args" )
    {
        auto apply = init_tag(R"(
		<ml:apply>
			<ml:dunno/>
            <ml:id>hello</ml:id>
            <ml:id>hello</ml:id>
		</ml:apply>
        )");
        REQUIRE( sv(apply.name()) == "ml:apply");
        REQUIRE( sv(apply.first_child().name()) == "ml:dunno");

        SECTION( "matlab" )
        {
            matlab::convert(apply, os);
            REQUIRE( os.str() == "'apply' contains <ml:dunno> with two arguments <ml:id>, <ml:id>\n" );
        }
    }
    SECTION( "apply dunno three args" )
    {
        auto apply = init_tag(R"(
		<ml:apply>
			<ml:dunno/>
            <ml:id>hello</ml:id>
            <ml:id>hello</ml:id>
            <ml:real>0.039</ml:real>
		</ml:apply>
        )");
        REQUIRE( sv(apply.name()) == "ml:apply");
        REQUIRE( sv(apply.first_child().name()) == "ml:dunno");

        SECTION( "matlab" )
        {
            matlab::convert(apply, os);
            REQUIRE( os.str() == "'apply' contains <ml:dunno> with three (or more) arguments <ml:id>, <ml:id>, <ml:real>\n" );
        }
    }
    SECTION( "apply id four args" )
    {
        auto apply = init_tag(R"(
		<ml:apply>
			<ml:id>some_fun</ml:id>
            <ml:id>hello</ml:id>
            <ml:real>1</ml:real>
            <ml:id>there</ml:id>
            <ml:real>2</ml:real>
		</ml:apply>
        )");
        REQUIRE( sv(apply.name()) == "ml:apply");
        REQUIRE( sv(apply.first_child().name()) == "ml:id");

        SECTION( "matlab" )
        {
            matlab::convert(apply, os);
            REQUIRE( os.str() == "some_fun(hello, 1, there, 2)" );
        }
    }
    SECTION( "apply dunno no args" )
    {
        auto apply = init_tag(R"(
		<ml:apply>
			<ml:dunno/>
		</ml:apply>
        )");
        REQUIRE( sv(apply.name()) == "ml:apply");
        REQUIRE( sv(apply.first_child().name()) == "ml:dunno");

        SECTION( "matlab" )
        {
            matlab::convert(apply, os);
            REQUIRE( os.str() == "'apply' contains <ml:dunno> and no other tags\n" );
        }
    }
    SECTION( "apply tan" )
    {
        auto apply = init_tag(R"(
		<ml:apply>
			<ml:id xml:space="preserve">tan</ml:id>
			<ml:id xml:space="preserve" subscript="b">a</ml:id>
		</ml:apply>
        )");
        REQUIRE( sv(apply.name()) == "ml:apply");
        REQUIRE( sv(apply.first_child().name()) == "ml:id");

        SECTION( "matlab" )
        {
            matlab::convert(apply, os);
            REQUIRE( os.str() == "tan(a_b)" );
        }
    }
    SECTION( "unitReference" )
    {
        auto apply = init_tag(R"(<unitReference unit="radian"/>)");
        REQUIRE( sv(apply.name()) == "unitReference");

        SECTION( "matlab" )
        {
            matlab::convert(apply, os);
            REQUIRE( os.str() == "radian" );
        }
    }
    SECTION( "unitReference w/ power-numerator" )
    {
        auto apply = init_tag(R"(<unitReference unit="second" power-numerator="-2"/>)");
        REQUIRE( sv(apply.name()) == "unitReference");

        SECTION( "matlab" )
        {
            matlab::convert(apply, os);
            REQUIRE( os.str() == "second^-2" );
        }
    }

}