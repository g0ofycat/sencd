#include "session_manager.h"

//--============
// -- LOGIC
//--============

void session_manager_init(SESSION_MANAGER_T *manager) {
	memset(manager->sessions, 0, sizeof(manager->sessions));
	manager->session_count = 0;
	pthread_mutex_init(&manager->lock, NULL);
}

SESSION_T *session_manager_connect(SESSION_MANAGER_T *manager, SESSION_ROLE_T role, int socket, const char *ip) {
	SESSION_T *session = malloc(sizeof(*session));

	if (session == NULL)
		return NULL;

	if (session_create(session, role, socket, ip) != 0) {
		free(session);
		return NULL;
	}

	if (session_server_connect(session) != 0) {
		session_destroy(session);
		free(session);
		return NULL;
	}

	if (session_manager_add(manager, session) != 0) {
		session_destroy(session);
		free(session);
		return NULL;
	}

	return session;
}

int session_manager_disconnect(SESSION_MANAGER_T *manager, uint64_t session_id) {
	return session_manager_remove(
			manager,
			session_id
			);
}

void session_manager_destroy(SESSION_MANAGER_T *manager) {
	pthread_mutex_lock(&manager->lock);

	for (uint32_t i = 0; i < MAX_SESSIONS; i++) {
		if (manager->sessions[i] != NULL) {
			session_destroy(manager->sessions[i]);
			free(manager->sessions[i]);
			manager->sessions[i] = NULL;
		}
	}

	manager->session_count = 0;

	pthread_mutex_unlock(&manager->lock);
	pthread_mutex_destroy(&manager->lock);
}

int session_manager_add(SESSION_MANAGER_T *manager, SESSION_T *session) {
	pthread_mutex_lock(&manager->lock);

	for (uint32_t i = 0; i < MAX_SESSIONS; i++) {
		if (manager->sessions[i] == NULL) {
			manager->sessions[i] = session;
			manager->session_count++;

			pthread_mutex_unlock(&manager->lock);
			return 0;
		}
	}

	pthread_mutex_unlock(&manager->lock);

	return 1;
}

int session_manager_remove(SESSION_MANAGER_T *manager, uint64_t session_id) {
	pthread_mutex_lock(&manager->lock);

	for (uint32_t i = 0; i < MAX_SESSIONS; i++) {
		SESSION_T *session = manager->sessions[i];
		if (session != NULL && session->session_id == session_id) {
			session_destroy(session);
			free(session);

			manager->sessions[i] = NULL;
			manager->session_count--;

			pthread_mutex_unlock(&manager->lock);
			return 0;
		}
	}

	pthread_mutex_unlock(&manager->lock);

	return 1;
}

SESSION_T *session_manager_get(SESSION_MANAGER_T *manager, uint64_t session_id) {
	pthread_mutex_lock(&manager->lock);

	for (uint32_t i = 0; i < MAX_SESSIONS; i++) {
		SESSION_T *session = manager->sessions[i];
		if (session != NULL && session->session_id == session_id) {
			pthread_mutex_unlock(&manager->lock);
			return session;
		}
	}

	pthread_mutex_unlock(&manager->lock);

	return NULL;
}
