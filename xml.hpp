// Joseph Prichard
// 4/25/2024
// Xml parsing library for C++

#include <string>
#include <vector>
#include <variant>
#include <memory>
#include <optional>
#include <sstream>
#include <cstring>

namespace xmlc {
    // an attribute in a xmlc element tag
    struct XmlAttr {
        std::pmr::string name;
        std::pmr::string value;

        [[nodiscard]] const char* get_name() const {
            return name.c_str();
        }

        [[nodiscard]] const char* get_value() const {
            return value.c_str();
        }

        friend bool operator==(const XmlAttr& attr, const XmlAttr& other) {
            return attr.name == other.name && attr.value == other.value;
        }
    };

    // raw text node in the xmlc hierarchy
    using XmlText = std::pmr::string;

    // a decl node to represent xmlc document metadata
    struct XmlDecl {
        std::pmr::string tag;
        std::pmr::vector<XmlAttr> attrs;

        friend bool operator==(const XmlDecl& decl, const XmlDecl& other) {
            return decl.tag == other.tag && decl.attrs == other.attrs;
        }
    };

    struct XmlNode;

    bool operator==(const XmlNode& node, const XmlNode& other);

    // an elem in the document hierarchy with a tag, atttributes, and other children as nodes
    struct XmlElem {
        std::pmr::string tag;
        std::pmr::vector<XmlAttr> attrs;
        std::pmr::vector<XmlNode*> children;

        const XmlElem* find_child(const char* ctag) const;
        const XmlAttr* find_attr(const char* attr_name) const;

        friend bool operator==(const XmlElem& elem, const XmlElem& other) {
            if (elem.tag != other.tag || elem.attrs != other.attrs) {
                return false;
            }

            if (elem.children.size() != other.children.size()) {
                return false;
            }
            for (int i = 0; i < elem.children.size(); i++) {
                if (*elem.children[i] != *other.children[i])
                    return false;
            }
            return true;
        }
    };

    // thrown when a node is not the expected type
    class NodeTypeException : public std::exception {
    private:
        std::string message;
    public:
        explicit NodeTypeException(const std::string& m) : message(m) {
            printf("%s\n", m.c_str());
        }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

    // a xmlc node in the document hierarchy
    struct XmlNode {
        std::variant<XmlElem, XmlText, XmlDecl> data;

        [[nodiscard]] bool is_text() const {
            return holds_alternative<XmlText>(data);
        }

        [[nodiscard]] bool is_elem() const {
            return holds_alternative<XmlElem>(data);
        }

        [[nodiscard]] const char* as_text() const {
            if (auto text = get_if<XmlText>(&data))
                return text->c_str();
            throw NodeTypeException("XML Node is not a text type node");
        }

        [[nodiscard]] const XmlElem* as_elem() const {
            if (auto elem = get_if<XmlElem>(&data))
                return elem;
            throw NodeTypeException("XML Node is not an elem type node");
        }

        [[nodiscard]] const XmlElem* find_child(const char* tag) const {
            if (auto elem = get_if<XmlElem>(&data))
                return elem->find_child(tag);
            return nullptr;
        }

        std::string serialize();

        friend bool operator==(const XmlNode& node, const XmlNode& other) {
            return node.data == other.data;
        }
    };

    // represents an entire parsed document
    struct XmlDocument {
        std::pmr::vector<XmlNode*> children;

        const XmlElem* find_child(const char* tag) const {
            for (auto child : children)
                if (auto elem = get_if<XmlElem>(&child->data))
                    if (strcmp(elem->tag.c_str(), tag) == 0)
                        return elem;
            return nullptr;
        }

        std::string serialize() {
            std::string str;
            for (auto node : children)
                str += node->serialize();
            return str;
        }

        friend bool operator==(const XmlDocument& document, const XmlDocument& other) {
            if (document.children.size() != other.children.size()) {
                return false;
            }
            for (int i = 0; i < document.children.size(); i++) {
                if (*document.children[i] != *other.children[i])
                    return false;
            }
            return true;
        }
    };

    // thrown when an exception is found during parsing
    class TokenException : public std::exception {
    private:
        std::string message;
    public:
        explicit TokenException(const std::string& m) : message(m) {
            printf("%s\n", m.c_str());
        }

        [[nodiscard]] const char* what() const noexcept override {
            return message.c_str();
        }
    };

    // parses a xmlc document from a string into a node hierarchy - all nodes are allocated within the provided memory resource
    XmlDocument parse_document(const std::string& docstr, std::pmr::memory_resource& resource);
}
