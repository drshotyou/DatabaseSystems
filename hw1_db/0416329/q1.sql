SELECT matchId, MAX(DBNOs) FROM player_statistics GROUP BY matchId ORDER BY MAX(DBNOs) DESC LIMIT 20;
