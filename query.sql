-- DELETE FROM preceptors WHERE id = 1;

-- UPDATE residents
-- SET preceptor_id = 2
-- WHERE id = 1;

-- INSERT INTO residents (name, password, preceptor_id, residency_id, frequency)
-- VALUES ('joao', '123', 3, 2, 0);

-- UPDATE residents
-- SET name = 'luis'
-- WHERE id = 2;

INSERT INTO activities_residents (activity_id, residency_id, user_id, done) VALUES (1, 1, 1, 1);