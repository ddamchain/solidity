#include <libsolidity/analysis/FunctionCallGraph.h>

#include <boost/range/adaptor/reversed.hpp>

using namespace solidity::frontend;

std::vector<std::unique_ptr<FCG const>> FunctionCallGraphBuilder::analyze()
{
	for (auto const* contract: m_currentContract->annotation().linearizedBaseContracts  | boost::adaptors::reversed)
	{
		contract->constructor()->accept(*this);
	}

	for (auto const* stateVar: m_currentContract->stateVariablesIncludingInherited())
	{
		m_currentFCG = std::make_unique<FCG>(stateVar);
		m_currentFCG->writes.emplace_back(stateVar);
		stateVar->value()->accept(*this);
		m_rootFCGs.push_back(std::move(m_currentFCG));
	}

	return std::move(m_rootFCGs);
}

bool FunctionCallGraphBuilder::visit(FunctionDefinition const& _functionDefinition)
{
	std::unique_ptr<FCG> previousFCG = move(m_currentFCG);

	m_currentFCG = std::make_unique<FCG>(&_functionDefinition);
	_functionDefinition.accept(*this);

	// No previous fcg so this is a root fcg (constructor)
	if (!previousFCG)
		m_rootFCGs.push_back(std::move(m_currentFCG));
	else
	{
		previousFCG->calls.emplace_back(std::move(m_currentFCG));
		m_currentFCG = std::move(previousFCG);
	}

	return false;
}

bool FunctionCallGraphBuilder::visit(FunctionCall const& _functionCall)
{
	_functionCall.expression().accept(*this);

	if (FunctionTypePointer functionPointer = dynamic_cast<FunctionTypePointer>(_functionCall.annotation().type))
		if (FunctionDefinition const* functionDefinition = dynamic_cast<decltype(functionDefinition)>(&functionPointer->declaration()))
		{
			findFinalOverride(functionDefinition)->accept(*this);
		}

	return false;
}


bool FunctionCallGraphBuilder::visit(Identifier const& _identifier)
{
	if (auto const variableDef = dynamic_cast<VariableDeclaration const*>(_identifier.annotation().referencedDeclaration))
		if (variableDef->isStateVariable())
			m_currentFCG->reads.emplace_back(variableDef);

	return false;
}


bool FunctionCallGraphBuilder::visit(Assignment const& _assignment)
{
	_assignment.rightHandSide().accept(*this);

	if (auto const ident = dynamic_cast<Identifier const*>(&_assignment.leftHandSide()))
		if (auto const variableDef = dynamic_cast<VariableDeclaration const*>(ident->annotation().referencedDeclaration))
			if (variableDef->isStateVariable())
				m_currentFCG->writes.emplace_back(variableDef);

	return false;
}


FunctionDefinition const* FunctionCallGraphBuilder::findFinalOverride(FunctionDefinition const* _functionDefinition)
{
	if (!_functionDefinition->virtualSemantics())
		return _functionDefinition;

	for (auto const* contract: m_currentContract->annotation().linearizedBaseContracts)
		for (auto const funcDef: contract->definedFunctions())
			if (funcDef->name() == _functionDefinition->name())
			{
				FunctionTypePointer fpA = FunctionType(*funcDef).asCallableFunction(false);
				FunctionTypePointer fpB = FunctionType(*_functionDefinition).asCallableFunction(false);
				if (fpA->hasEqualReturnTypes(*fpB) && fpA->hasEqualParameterTypes(*fpB))
					return funcDef;
			}

	return _functionDefinition;
}
