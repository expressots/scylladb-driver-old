<h1 align="center"> ScyllaDB Driver </h1>

> The native ScyllaDB Driver wrapper made by @ExpressoTS Team for the Node.js community ❤🚀.

# Getting Started

First, make sure you have the driver properly [installed](https://github.com/expressots/scylladb-driver/...).

## Connecting to a Cluster

Before we can start executing any queries against a Cassandra cluster we need to setup 
an instance of Cluster. As the name suggests, you will typically have one instance of 
Cluster for each Cassandra cluster you want to interact with.

### Connecting to Cassandra

The simplest way to create a Cluster is like this:

```ts
import { Cluster } from "scylladb-driver"

const cluster = new Cluster();
```

This will attempt to connection to a Cassandra instance on your local machine (127.0.0.1).
You can also specify a list of IP addresses for nodes in your cluster:

```ts
import { Cluster } from "scylladb-driver"

const cluster = new Cluster({
  nodes: ["192.168.0.1", "192.168.0.2"]
});
```

The set of IP addresses we pass to the `Cluster` is simply an initial set of contact points. 
After the driver connects to one of these nodes it will _automatically_ discover the rest of the 
nodes in the cluster and connect to them, so you don’t need to list every node in your cluster.

If you need to use a non-standard port, use SSL, or customize the driver’s behavior in some other way, this is the place to do it:

```ts
import { Cluster } from "scylladb-driver"

const cluster = new Cluster({
  nodes: ["192.168.0.1", "192.168.0.2"],
  port: 1234,
  sslContext: // TODO: Understand what would this be.
});
```

Instantiating a `Cluster` does **not** actually connect us to any nodes. 
To establish connections and begin executing queries we need a `Session`, which is created by 
calling the `connect()` method from the `Cluster` class.

```ts
import { Cluster } from "scylladb-driver"

const cluster = new Cluster();
const session = cluster.connect();
```

## Session keyspace

The `connect()` method takes an optional `keyspace` argument which sets the default keyspace 
for all queries made through that `Session`:

```ts
import { Cluster } from "scylladb-driver"

const cluster = new Cluster();
const session = cluster.connect("keyspace");
```

You can always change a `Session`’s `keyspace` using `setKeyspace()` or by executing a `USE <keyspace>` query:

```ts
const keyspace = "keyspace"
session.setKeyspace(keyspace);
// Or you can do this instead
session.query(`USE ${keyspace}`);
```

## Executing Queries

Now that we have a `Session` we can begin to execute queries. The simplest way to execute a query is to use `execute()`:

```ts
type UserTable = {
  name: string;
  age: number;
  email: string;
};

const rows: UserTable = session.execute("SELECT name, age, email FROM users");
rows.forEach(({ name, age, email }) => {
  console.log(`${name}, ${age}, ${email}`);
});
```
