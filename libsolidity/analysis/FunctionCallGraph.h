/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <libsolidity/ast/ASTVisitor.h>
#include <memory>

namespace solidity::frontend
{

struct FCG
{
	FCG(Declaration const* _definition): definition(_definition) {}

	Declaration const* definition;
	std::vector<std::unique_ptr<FCG>> calls;
	std::vector<std::shared_ptr<VariableDeclaration const>> reads;
	std::vector<std::shared_ptr<VariableDeclaration const>> writes;
};

class FunctionCallGraphBuilder : private ASTConstVisitor
{
public:
	FunctionCallGraphBuilder(ContractDefinition const* _contractDefinition):
		m_currentContract(_contractDefinition) {}

	std::vector<std::unique_ptr<FCG const>> analyze();

private:
	bool visit(FunctionDefinition const& _functionDefinition);
	bool visit(FunctionCall const& _functionCall);
	bool visit(Identifier const& _identifier);
	bool visit(Assignment const& _assignment);

	FunctionDefinition const* findFinalOverride(FunctionDefinition const* _functionDefinition);

	ContractDefinition const* m_currentContract;
	std::unique_ptr<FCG> m_currentFCG;
	std::vector<std::unique_ptr<FCG const>> m_rootFCGs;
};

}
