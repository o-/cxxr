#ifndef PROVENANCE_HPP
#define PROVENANCE_HPP

#ifdef __cplusplus

#include <sys/time.h>
#include <ctime>
#include <set>
#include <boost/serialization/access.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/export.hpp>
#include "CXXR/Expression.h"
#include "CXXR/GCEdge.hpp"
#include "CXXR/GCNode.hpp"
#include "CXXR/GCStackRoot.h"
#include "CXXR/RObject.h"
#include "CXXR/StringVector.h"
#include "CXXR/Symbol.h"

namespace CXXR {
	class Parentage;

	class Provenance : public GCNode {
	public:
		class CompTime {
		public:
			bool operator()(Provenance* lhs, Provenance* rhs) {
				return (lhs->m_timestamp.tv_sec==rhs->m_timestamp.tv_sec) ?
					(lhs->m_timestamp.tv_usec<rhs->m_timestamp.tv_usec) :
					(lhs->m_timestamp.tv_sec<rhs->m_timestamp.tv_sec);
			}
		};
		typedef std::set<Provenance*,Provenance::CompTime> Set;

		Provenance(); // sort of for boost::serialization
		Provenance(Expression*,Symbol*,Parentage*);
		~Provenance();

		static Set* ancestors(Set*);
		static Set* descendants(Set*);
		static GCStackRoot<StringVector> setAsStringVector(Set*);

		Set* children() const;
		void detachReferents();
		Expression* getCommand() const;
		Symbol* getSymbol() const;
		Parentage* getParentage() const;
		const CachedString* getTime() const;
		Set* pedigree();
		void visitReferents(const_visitor*) const;
	private:
		friend class boost::serialization::access;
		struct timeval m_timestamp;
		unsigned int m_parentpos;
		Set* m_children;
		GCEdge<Expression> m_expression;
		GCEdge<Symbol> m_symbol;
		Parentage* m_parentage;

		void announceBirth();
		void announceDeath();
		void deregisterChild(Provenance*);
		void registerChild(Provenance*);
		template <class Archive>
		void serialize(Archive & ar, const unsigned int version) {
			ar & boost::serialization::base_object<GCNode>(*this);
			printf("Serialize Provenance\n");
			ar & m_parentpos;
		}
	};
} // Namespace CXXR

BOOST_CLASS_EXPORT(CXXR::Provenance)

#endif
#endif
