import { Cluster } from "scylladb-driver";

const cluster = new Cluster({
  nodes: [process?.env.host ?? "127.0.0.1"],
});

const session = cluster.connect();

function createKeyspace() {
  session.executeSync(
    `CREATE KEYSPACE IF NOT EXISTS driver_test WITH REPLICATION = { 'class' : 'SimpleStrategy', 'replication_factor' : 1 }`
  );
}

// Create a simple table for user <name, age, email>
function createTable() {
  session.executeSync(`
    CREATE TABLE IF NOT EXISTS users (
      name text,
      age int,
      email text,
      PRIMARY KEY (name)
    );
  `);
}

// Insert a new user if the user does not exist
type User = {
  name: string;
  age: number;
  email: string;
};

function insertUser(user: User) {
  session.executeSync(`
    INSERT INTO users (name, age, email)
    VALUES ('${user.name}', ${user.age}, '${user.email}')
    IF NOT EXISTS;
  `);
}

function selectUser(name: string) {
  const result = session.executeSync(`
    SELECT * FROM users WHERE name = '${name}';
  `);
  return result[0];
}

function selectAllUsers() {
  const result = session.executeSync(`
    SELECT * FROM users;
  `);
  return result;
}

export function run() {
  createKeyspace();
  session.executeSync("USE driver_test");
  createTable();

  [
    {
      name: "John",
      age: 30,
      email: "john@doe.com",
    },
    {
      name: "Jane",
      age: 25,
      email: "jane@doe.com",
    },
  ].forEach(insertUser);

  selectAllUsers().forEach(({ name, age, email }) => {
    console.log(`${name} is ${age} years old and has email ${email}`);
  });
}
