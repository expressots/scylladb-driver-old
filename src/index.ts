import { Cluster } from "@nodepp/example";

const cluster = new Cluster({
  nodes: ["172.22.0.2"]
});

const session = cluster.connect();
const keyspaceName = session.executeSync("SELECT keyspace_name FROM system_schema.keyspaces");

// const keyspaceName = session.executeSync("SELECT keyspace_name FROM system_schema.keyspaces"); // block my execution thread here.
// console.log(keyspaceName); // system_auth
