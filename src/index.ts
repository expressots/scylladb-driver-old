import { Cluster } from "@nodepp/example";

const cluster = new Cluster({
  nodes: ["172.21.0.2"],
});

const session = cluster.connect();
const keyspaceName = session.executeSync(
  "SELECT table_name FROM system_schema.scylla_tables"
);
console.log(keyspaceName);
