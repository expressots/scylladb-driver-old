import { Cluster } from "@nodepp/example";

const cluster = new Cluster({
  nodes: [process?.env.host ?? "127.0.0.1"],
});

export function run() {
  const session = cluster.connect("system_schema");
  const keyspaceName = session.executeSync<{ table_name: string }>(
    "SELECT table_name FROM scylla_tables"
  );
  console.log(keyspaceName);
}
