int leggi(Point p){
	struct sembuf writer, reader;
	writer.sem_num = p.y*SO_WIDTH + p.x;
	writer.sem_op = 0;
	writer.sem_flg = 0;
	reader.sem_num = p.y*SO_WIDTH + p.x;
	reader.sem_op = 1;
	reader.sem_flg = 0;
	return semop(sem_idW, &writer, 1) + semop(sem_idR, &reader, 1);
}

int scrivi(Point p){
	/*semctl(sem_idW, p.y*SO_WIDTH + p.x, GETZCNT, &idR);*/
	struct sembuf writer[2], reader[2];
	writer[0].sem_num = p.y*SO_WIDTH + p.x;
	writer[0].sem_op = 0;
	writer[0].sem_flg = 0;
	reader[0].sem_num = p.y*SO_WIDTH + p.x;
	reader[0].sem_op = 0;
	reader[0].sem_flg = 0;
	writer[1].sem_num = p.y*SO_WIDTH + p.x;
	writer[1].sem_op = 1;
	writer[1].sem_flg = 0;
	reader[1].sem_num = p.y*SO_WIDTH + p.x;
	reader[1].sem_op = 1;
	reader[1].sem_flg = 0;
	return semop(sem_idW, writer, 1) + semop(sem_idR, reader, 2);
	
}

int releaseW (Point p){
	struct sembuf releaseW, releaseR;
	releaseW.sem_num = p.y*SO_WIDTH + p.x;
	releaseW.sem_op = -1;
	releaseW.sem_flg = IPC_NOWAIT;
	releaseR.sem_num = p.y*SO_WIDTH + p.x;
	releaseR.sem_op = -1;
	releaseR.sem_flg = IPC_NOWAIT;
	return semop(sem_idW, &releaseW,1) + semop(sem_idR, &releaseR, 1);
}

int releaseR (Point p){
	struct sembuf releaseR;
	releaseR.sem_num = p.y*SO_WIDTH + p.x;
	releaseR.sem_op = -1;
	releaseR.sem_flg = IPC_NOWAIT;
	return semop(sem_idR, &releaseR, 1);
}
