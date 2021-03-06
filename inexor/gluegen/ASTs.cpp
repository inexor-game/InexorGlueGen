
#include "inexor/gluegen/ASTs.hpp"

#include <boost/algorithm/string.hpp>

#include <vector>
#include <iostream>

using namespace std;
using namespace pugi;
using namespace inexor::filesystem;
using namespace boost;

namespace inexor {
namespace gluegen {


/// Returns true if this compound xml node has as parent the SharedOption base class.
bool is_option_class_node(const xml_node &class_compound_xml)
{
    for(xml_node &base_class : class_compound_xml.children("basecompoundref"))
    {
        string baseclassname = base_class.text().as_string();
        if(baseclassname == "SharedOption")
            return true;
    }
    return false;
}

void ASTs::load_from_directory(const Path &directory)
{
    std::vector<Path> all_xml_file_names;
    list_files(directory, all_xml_file_names, ".xml");
    for(auto &file : all_xml_file_names)
    {
        file.make_preferred();

        // handle code ASTs
        if(contains(file.filename().string(), "_8cpp.xml") || contains(file.filename().string(), "_8hpp.xml")
           || contains(file.filename().string(), "namespace"))// cpp/hpp files for namespaced contents
        {
            auto xml = make_unique<xml_document>();
            if(load_xml_file(file, xml)) code_xmls.push_back(std::move(xml));
            continue;
        }

        if(!contains(file.stem().string(), "class") && !contains(file.stem().string(), "struct"))
            continue;
        // handle class ASTs:
        // either a SharedOption (remember it by classname) or just remember the class AST by its refid (doxygens reference ID)

        auto xml = make_unique<xml_document>();
        if(!load_xml_file(file, xml)) continue;

        const xml_node compound_xml = xml->child("doxygen").child("compounddef"); //[@kind='class' and @language='C++']");

        if(is_option_class_node(compound_xml)) {
            attribute_class_xmls.push_back(std::move(xml));
        }
        else {
            class_xmls[compound_xml.attribute("id").value()] = std::move(xml);
        }
    }
}

bool ASTs::load_xml_file(const Path &file, unique_ptr<xml_document>& xml)
{
    if(!xml->load_file(file.c_str(), parse_default|parse_trim_pcdata))
    {
        std::cout << "XML file representing the AST couldn't be parsed: " << file << std::endl;
        return false;
    }
    return true;
}


} } // ns inexor::gluegen
