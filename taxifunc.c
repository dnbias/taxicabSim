int leggi(Point p){
	struct sembuf mutexW, reader;
	/*c'è uno scrittore?*/
	mutexW.sem_num = p.y*SO_WIDTH + p.x;
	mutexW.sem_op = 0; 
	mutexW.sem_flg = 0;
	/*dico che sto scrivendo*/
	reader.sem_num = p.y*SO_WIDTH + p.x;
	reader.sem_op = 1;
	reader.sem_flg = 0;
	return semop(sem_idW, &mutex, 1) + semop(sem_idR, &reader, 1);
}

int scrivi(Point p){
	/*semctl(sem_idW, p.y*SO_WIDTH + p.x, GETZCNT, &idR);*/
	struct sembuf mutexW[2], reader/*[2]*/;
	/*c'è uno scrittore?*/
	mutexW[0].sem_num = p.y*SO_WIDTH + p.x;
	mutexW[0].sem_op = 0;
	mutexW[0].sem_flg = 0;
	/*ci sono dei lettori?*/
	reader.sem_num = p.y*SO_WIDTH + p.x;
	reader.sem_op = 0;
	reader.sem_flg = 0;
	/*blocco i prossimi scrittori e lettori*/
	mutexW[1].sem_num = p.y*SO_WIDTH + p.x;
	muetxW[1].sem_op = 1;
	mutexW[1].sem_flg = 0;
	/*reader[1].sem_num = p.y*SO_WIDTH + p.x;
	reader[1].sem_op = 1;
	reader[1].sem_flg = 0;*/
	return semop(sem_idW, mutex, 2) + semop(sem_idR, &reader, 1);
	
}

int releaseW (Point p){
	struct sembuf releaseW, /*releaseR*/;
	releaseW.sem_num = p.y*SO_WIDTH + p.x;
	releaseW.sem_op = -1;
	releaseW.sem_flg = IPC_NOWAIT;
	/*releaseR.sem_num = p.y*SO_WIDTH + p.x;
	releaseR.sem_op = -1;
	releaseR.sem_flg = IPC_NOWAIT;*/
	return semop(sem_idW, &releaseW,1) + semop(sem_idR, &releaseR, 1);
}

int releaseR (Point p){
	struct sembuf reader;
	reader.sem_num = p.y*SO_WIDTH + p.x;
	reader.sem_op = -1;
	reader.sem_flg = IPC_NOWAIT;
	return semop(sem_idR, &reader, 1);
}
