#include <catch2/catch_test_macros.hpp>
#include "pugixml.hpp"
#include "matlab.hpp"
#include <sstream>
#include <iostream>

using sv = std::string_view;

TEST_CASE( "simple xml tags" )
{
	pugi::xml_document doc;

    auto init_tag = [&doc](const sv xml){
        std::string doc_tag = "<?xml?>";
        doc_tag += xml;
        auto result = doc.load_string(doc_tag.c_str());
        REQUIRE( result );
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
    SECTION( "apply mult ")
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