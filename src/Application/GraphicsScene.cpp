#include "GraphicsScene.h"
#include "IGraphicsNode.h" 

using namespace cadutils;
using namespace std;


shared_ptr<IGraphicsNode> cadutils::GraphicsScene::GetOrCreate(ObjectId id)
{
	auto itemIter = m_items.find(id);
	if (itemIter != m_items.end())
	{
		return itemIter->second;
	}

	shared_ptr<IGraphicsNode> newNode = IGraphicsNode::CreateGraphicsNode(id);
	m_items[id] = newNode;
	return newNode;
}

std::shared_ptr<IGraphicsNode> cadutils::GraphicsScene::Find(ObjectId id) const
{
	auto itemIter = m_items.find(id);
	return itemIter != m_items.end() ? itemIter->second : nullptr;
}

std::vector<std::shared_ptr<IGraphicsNode>> cadutils::GraphicsScene::GetItems() const
{
	vector<std::shared_ptr<IGraphicsNode>> allNodes;
	for (const auto& item : m_items)
	{
		allNodes.emplace_back(item.second);
	}
	return allNodes;
}

const std::unordered_map<ObjectId, std::shared_ptr<IGraphicsNode>>& cadutils::GraphicsScene::GetAllNodesWithId() const
{
	return m_items;
}

void cadutils::GraphicsScene::Remove(ObjectId id)
{
	auto idIter = m_items.find(id);
	if (idIter != m_items.end())
	{
		m_items.erase(idIter);
	}
}
