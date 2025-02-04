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
    SECTION( "apply mult" )
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
//					<ml:parens>
//						<ml:apply>
//							<ml:minus/>
//							<ml:apply>
//								<ml:pow/>
//								<ml:id xml:space="preserve">e</ml:id>
//								<ml:apply>
//									<ml:div/>
//									<ml:id xml:space="preserve">VD</ml:id>
//									<ml:apply>
//										<ml:mult/>
//										<ml:real>0.039</ml:real>
//										<ml:id xml:space="preserve">V</ml:id>
//									</ml:apply>
//								</ml:apply>
//							</ml:apply>
//							<ml:real>1</ml:real>
//						</ml:apply>
//					</ml:parens>
}