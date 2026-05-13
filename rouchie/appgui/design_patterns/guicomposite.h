#pragma once

#include "../guibase.h"

#include <map>
#include <vector>

// 树形结构可以使用组合模式

class FileSystemNode {
public:
    using Ptr = std::shared_ptr<FileSystemNode>;
public:
    explicit FileSystemNode(const std::string &name, Ptr parent)
        : _name(name), _parent(parent) { }
    virtual ~FileSystemNode() = default;

    virtual void add(const Ptr &node) { throw std::runtime_error("Not support"); }
    virtual void del(const Ptr &node) { throw std::runtime_error("Not support"); }

    virtual size_t size() = 0;
    virtual std::string name() { return _name; }

    virtual std::vector<Ptr> subNode() const { throw std::runtime_error("Not support"); }
    virtual bool hasSubNode() { return false; }

    Ptr parentNode() { return _parent; }

private:
    std::string _name;
    Ptr _parent;
};

class File : public FileSystemNode {
public:
    explicit File(const std::string &name, const size_t size, const Ptr &parent)
        : FileSystemNode(name, parent)
        , _size(size) { }
    size_t size() override { return _size; }

private:
    size_t _size;
};

class Folder : public FileSystemNode {
public:
    explicit Folder(const std::string &name, const Ptr &parent)
        : FileSystemNode(name, parent) { }

    void add(const Ptr &node) override { _nodes.emplace(node->name(), node); }
    void del(const Ptr &node) override { _nodes.erase(node->name()); }

    size_t size() override {
        size_t size = 0;
        for (const auto &node : _nodes) {
            size += node.second->size();
        }
        return size;
    }

    std::vector<Ptr> subNode() const override {
        std::vector<Ptr> v;
        for (const auto & node : _nodes) {
            v.emplace_back(node.second);
        }
        return v;
    }
    bool hasSubNode() override { return true; }

private:
    std::map<std::string, Ptr> _nodes;
};

class GuiComposite : public GuiBase {
public:
    GuiComposite();
    void operator()() override;

private:
    FileSystemNode::Ptr _root;
    FileSystemNode::Ptr _current;
};

inline GuiComposite::GuiComposite() {
    _root = std::make_shared<Folder>("root", nullptr);
    _current = _root;
}

inline void GuiComposite::operator()() {
    ImGui::Begin("桥接");

    static std::string newNodeName;
    ImGui::InputText(u8"输入目录/文件名称", &newNodeName);

    if (ImGui::Button(u8"添加目录") && !newNodeName.empty()) {
        _current->add(std::make_shared<Folder>(newNodeName, _current));
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"添加文件") && !newNodeName.empty()) {
        _current->add(std::make_shared<File>(newNodeName, 1024, _current));
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"根目录")) {
        _current = _root;
    }

    ImGui::SameLine();

    if (ImGui::Button(u8"上级目录")) {
        const auto parent = _current->parentNode();
        if (parent) {
            _current = parent;
        }
    }

    ImGui::BeginChild("ListContainer", ImVec2(200, 400), true);
    for (const auto& item : _current->subNode()) {
        // auto name = fmt::format("{}:{}", item->hasSubNode() ? "目录" : "文件", item->name());
        auto name = item->name();
        static std::string selected_item;
        const auto flag = item->hasSubNode() ? ImGuiSelectableFlags_None : ImGuiSelectableFlags_Disabled;
        if (ImGui::Selectable(name.c_str(), selected_item == name, flag)) {
            selected_item = name;
            _current = item;
        }
    }
    ImGui::EndChild();

    ImGui::End();
}
