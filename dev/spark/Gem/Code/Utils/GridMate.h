#pragma once

// simple implementation of a setter for a super standard gridmate setup
#define SPARK_UPDATE_VALUE(comp,v) \
	if (m_ ## v == v) return; \
	m_ ## v = v; \
	if (m_replicaChunk && AzFramework::NetQuery::IsEntityAuthoritative(GetEntityId())) \
	{ \
		auto replicaChunk = static_cast<comp ## ReplicaChunk*>(m_replicaChunk.get()); \
		replicaChunk->m_ ## v.Set(m_ ## v); \
	}
