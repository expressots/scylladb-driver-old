import { Cluster } from "@nodepp/example";

type User = {
  name: string;
  age: number;
  email: string;
};

describe('cassValueToTsValue', () => {
  const cluster = new Cluster({
    nodes: [process?.env.host ?? "127.0.0.1"],
  });
  const session = cluster.connect();

  session.executeSync(`CREATE KEYSPACE IF NOT EXISTS driver_test WITH REPLICATION = { 'class' : 'SimpleStrategy', 'replication_factor' : 1 };`);
  session.executeSync("USE driver_test;");

  test('should cluster has connect property and is instanceof Cluster', () => {
    expect(cluster instanceof Cluster).toEqual(true);
    expect(cluster).toHaveProperty('connect');
  });

  test('should session has executeSync property', () => {
    expect(session).toHaveProperty('executeSync');
  });

  test('testing types', () => {
    session.executeSync(`DROP TABLE IF EXISTS expressots_types;`);

    session.executeSync(`CREATE TABLE IF NOT EXISTS expressots_types (
      expressots_text text,
      expressots_varchar varchar,
      expressots_int int,
      expressots_boolean boolean,
      expressots_uuid uuid,
      expressots_bigint bigint,
      expressots_float float,
      expressots_double double,
      expressots_timestamp timestamp,
      expressots_timestamp2 timestamp,
      PRIMARY KEY (expressots_text)
    );`);

    session.executeSync(`INSERT INTO expressots_types (
      expressots_text,
      expressots_varchar,
      expressots_int,
      expressots_boolean,
      expressots_uuid,
      expressots_bigint,
      expressots_float,
      expressots_double,
      expressots_timestamp,
      expressots_timestamp2
      ) VALUES (
        'Expresso TS',
        'other',
        2,
        true,
        1e2384f0-299e-470b-91e7-c22a31cde59d,
        9223372036854775807,
        0.123,
        1.92233720,
        1681584477,
        '2011-02-03 04:05+0000'
      );`);

    const result = session.executeSync(`SELECT * FROM expressots_types WHERE expressots_text = 'Expresso TS';`);

    console.log(result[0])
    expect(typeof result[0].expressots_text).toBe('string')
    expect(typeof result[0].expressots_varchar).toBe('string')
    expect(typeof result[0].expressots_int).toBe('number')
    expect(typeof result[0].expressots_boolean).toBe('boolean')
    expect(typeof result[0].expressots_uuid).toBe('string')
    expect(typeof result[0].expressots_bigint).toBe('bigint')
    expect(typeof result[0].expressots_float).toBe('number')
    expect(typeof result[0].expressots_double).toBe('number')
    expect(typeof result[0].expressots_timestamp).toBe('number')
    expect(typeof result[0].expressots_timestamp2).toBe('number')
  });
});
